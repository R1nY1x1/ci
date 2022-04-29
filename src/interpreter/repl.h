#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "lexer.h"
#include "grid.h"
#include "renderer.h"
#include "../visualizer.h"

#include <ncurses.h>

void REP(visualizer *v, char *cmd) {
  /*
  Lexer l = newLexer(cmd);
  if (l.NextToken(&l).Type == SET) {
    Token tok = l.NextToken(&l);
    if ((strcmp(tok.Literal, "scale") == 0)) {
      if (l.NextToken(&l).Type == ASSIGN) {
        Token tok = l.NextToken(&l);
        for (int i = 0; i < (v->m->dim+1); i++) {
          v->figures[i].scale = atof(tok.Literal);
        }
      }
    } else if (strcmp(tok.Literal, "max_step") == 0) {
      if (l.NextToken(&l).Type == ASSIGN) {
        Token tok = l.NextToken(&l);
        for (int i = 0; i < (v->m->dim+1); i++) {
          v->figures[i].max_step = atof(tok.Literal);
        }
      }
    } else if (strcmp(tok.Literal, "run_by_step") == 0) {
      if (l.NextToken(&l).Type == ASSIGN) {
        v->run_by_step = (bool)atoi(l.NextToken(&l).Literal);
      }
    }
  }
  */
}
  
void REPL(void){
  char cmd[32];
  char prompt[4] = ">> ";

  while (1) {
    printf("%s", prompt);
    scanf("%[^\n]%*c", cmd);

    Lexer l = newLexer(cmd);
    Token tok;
    tok = l.NextToken(&l);
    while (tok.Type != EndOfFile) {
      printf("{%s, %s}\n", tok.TypeStr, tok.Literal);
      tok = l.NextToken(&l);
    }
  }
}
