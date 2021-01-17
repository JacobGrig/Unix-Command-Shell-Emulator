#pragma once

#include <unistd.h>

#include "analisys.h"

void SigHndl (int);
int shell (tree *);
int execute_cmd (const cmd_inf *);
