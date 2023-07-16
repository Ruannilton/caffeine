#include <core/caffeine_logging.h>

int main(int argc, char **argv) {
  caff_log_trace("Tracing\n");
  caff_log_info("Hello %s\n", "World");
  return 0;
}