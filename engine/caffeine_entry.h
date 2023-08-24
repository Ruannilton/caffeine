#pragma once

#include "core/caffeine_logging.h"
#include "core/caffeine_memory.h"

extern int caff_main(int argc, char **argv);

int main(int argc, char **argv){
  int client_response = 0;

  cff_memory_init();
  caff_log_init();

  client_response = caff_main(argc,argv);
  
  caff_log_end();
  cff_memory_end();
  
  return client_response;
}