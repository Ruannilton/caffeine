#include <caffeine_entry.h>

int main()
{
  bool app_started = caffeine_application_init("Caffeine");

  if (!app_started)
    return 1;

  bool sucess_exit = caffeine_application_run();

  return sucess_exit ? 0 : 2;
}