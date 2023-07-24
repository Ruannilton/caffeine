#include <core/caffeine_logging.h>
#include <core/caffeine_memory.h>

int main(int argc, char **argv) {
  cff_memory_init();
  caff_log_init();
  caff_log_error("ERROR\n");
  caff_log_warn("Warning\n");
  caff_log_trace("Tracing\n");
  caff_log_info("Hello %s\n", "World");
  caff_log_debug("Hoy Hey\n");
  caff_log_end();
  cff_memory_end();
  return 0;
}