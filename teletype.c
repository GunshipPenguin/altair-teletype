#include "i8080.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termio.h>
#include <ctype.h>
#include <sys/select.h>

const int PAUSE_CYCLES = 250000;
const int CLOCK_RATE = 2000000; // 2 MHz

int control_device;
int io_device;

int stdin_ready() {
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10;

  fd_set fds;
  FD_SET(fileno(stdin), &fds);

  if (select(fileno(stdin) + 1, &fds, NULL, NULL, &timeout) == -1) {
    perror("select");
    exit(1);
  }

  return FD_ISSET(fileno(stdin), &fds);
}

char read_tty_char() {
  char inp = getchar();

  if (inp == '\n') {
    inp = '\r';
  }

  return toupper(inp);
}

// 88-SIO emulation callback
uint handle_input_sio(struct i8080 *cpu, uint dev) {
  if (dev == io_device) {
    return stdin_ready() ? read_tty_char() : 0;
  } else if (dev == control_device) {
    return stdin_ready() ? 0 : 1;
  } else if (dev == 255) {
    return 0;
  }
}

// 88-2SIO emulation callback
uint handle_input_2sio(struct i8080 *cpu, uint dev) {
  if (dev == io_device) {
    // If stdin is not ready, return null byte, otherwise, read one char from stdin
    return !stdin_ready() ? 0x00 : read_tty_char();
  } else if (dev == control_device) {
    // If stdin is ready, set bit 0 to indicate the computer can read
    return stdin_ready() ? 0x03 : 0x02;
  } else if (dev == 0xFF) { // Altair sense switches
    // Required for I/O Altair I/O setup
    return 0x00;
  } else {
    fprintf(stderr, "Unknown device %d\n", dev);
    exit(1);
  }
}

void handle_output(struct i8080 *cpu, uint dev, uint val) {
  if (dev == io_device) {
    val &= 0x7F;
    if ((val >= 32 && val <= 125) || val == '\n') {
      printf("%c", val & 0x7F);
    }
    fflush(stdout);
  }
}

void sleep_for_clockrate(clock_t last_pause) {
  // Time taken in seconds since last pause
  double time_taken = (clock() - (double) last_pause) / CLOCKS_PER_SEC;

  // Time that should have been taken minus actual time taken
  double sleep_time_seconds = ((PAUSE_CYCLES / (double) CLOCK_RATE) - time_taken);

  struct timespec sleep_time;
  sleep_time.tv_sec = 0;
  sleep_time.tv_nsec = (long) (sleep_time_seconds * 1000000000);

  if (nanosleep(&sleep_time , NULL) < 0) {
    perror("nanosleep");
    exit(1);
  }
}

void init_termios() {
  struct termios term;

  if (tcgetattr(fileno(stdin), &term) != 0) {
    exit(1);
  }

  term.c_lflag &= ~(ECHO | ICANON);
  if (tcsetattr (fileno (stdin), TCSAFLUSH, &term) != 0) {
    exit(1);
  }
}

void parse_args(int argc, char *argv[], struct i8080 *cpu) {
  int c;
  char *file_name;
  size_t offset;
  while((c = getopt(argc, argv, "l:i:c:2")) != -1) {
    switch (c) {
      case 'l':
        file_name = optarg;

        if (optind >= argc) {
          fprintf(stderr, "No offset specified for file %s\n", file_name);
          exit(1);
        }

        offset = (size_t) strtol(argv[optind], NULL, 16);
        optind ++;
        i8080_load_memory(cpu, file_name, offset);
        break;
      case 'i':
        io_device = strtol(optarg, NULL, 10);
        break;
      case 'c':
        control_device = strtol(optarg, NULL, 10);
        break;
      case '2':
        cpu->input_handler = handle_input_2sio;
        break;
      case '?':
        exit(1);
      default:
        exit(1);
    }
  }
}

void run_cpu(struct i8080 *cpu) {
  clock_t last_pause;
  last_pause = clock();
  while (1) {
    i8080_step(cpu);

    if (cpu->cyc > PAUSE_CYCLES) {
      sleep_for_clockrate(last_pause);
      last_pause = clock();
      cpu->cyc = 0;
    }
  }
}

int main(int argc, char *argv[]) {
  init_termios();

  // Initialize cpu
  struct i8080 *cpu = malloc(sizeof(struct i8080));
  cpu->memsize = 65535;
  cpu->memory = malloc(cpu->memsize);
  i8080_reset(cpu);

  // Hook input/output to the correct card emulation
  cpu->input_handler = handle_input_sio;
  cpu->output_handler = handle_output;

  control_device = 0;
  io_device = 1;

  parse_args(argc, argv, cpu);

  run_cpu(cpu);
}