#include <core/caffeine_logging.h>

int main(int argc, char **argv) {
  caff_log_init();
  caff_log_error("ERROR\n");
  caff_log_warn("Warning\n");
  caff_log_trace("Tracing\n");
  caff_log_info("Hello %s\n", "World");
  caff_log_debug("Hoy Hey");
  caff_log_end();
  return 0;
}