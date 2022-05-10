#ifndef _REPL_H
#define _REPL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "token.h"
#include "lexer.h"
#include "grid.h"
#include "renderer.h"
#include "../visualizer.h"

#include <ncurses.h>

void REP(visualizer *v, char *cmd) {
  Lexer l = newLexer(cmd);
  Token tok = l.NextToken(&l);
  if (tok.Type == LET) {
    Token tok = l.NextToken(&l);
    if ((strcmp(tok.Literal, "scale") == 0)) {
      if (l.NextToken(&l).Type == ASSIGN) {
        Token tok = l.NextToken(&l);
        for (int i = 0; i < (v->vars_n); i++) {
          v->figures[i].scale = atof(tok.Literal);
        }
        v->textboxs[0].clear_t(&(v->textboxs[0]));
      }
    } else if (strcmp(tok.Literal, "max_step") == 0) {
      if (l.NextToken(&l).Type == ASSIGN) {
        Token tok = l.NextToken(&l);
        for (int i = 0; i < (v->vars_n); i++) {
          v->figures[i].max_step = atof(tok.Literal);
        }
        v->textboxs[0].clear_t(&(v->textboxs[0]));
      }
    } else if (strcmp(tok.Literal, "run_by_step") == 0) {
      if (l.NextToken(&l).Type == ASSIGN) {
        v->run_by_step = (bool)atoi(l.NextToken(&l).Literal);
      }
    }
  } else if (tok.Type == DRAW) {
    Token tok = l.NextToken(&l);
    for (int i = 0; i < v->vars_n; i++) {
      if (strcmp(tok.Literal, v->figures[i].var->name) == 0) {
        Token tok = l.NextToken(&l);
        if (strcmp(tok.Literal, "step") == 0) {
          if (l.NextToken(&l).Type == ASSIGN) {
            Token tok = l.NextToken(&l);
            char str[v->textboxs[0].column_n];
            //memset(str, ' ', sizeof(str));
            for (int y = v->figures[i].y+5; y < (v->figures[i].height + v->figures[i].y)+4; y++) {
              memset(str, ' ', sizeof(str));
              for(int i = 0; i < v->textboxs[0].column_n; i++) {
                if (v->textboxs[0].texts[y/4-1][i] != '\0') {
                  str[i] = v->textboxs[0].texts[y/4-1][i];
                }
              }
              str[fit_figure_x(atoi(tok.Literal), &(v->figures[i]))/2] = '|';
              v->textboxs[0].update(&v->textboxs[0], str, y/4+1);
            }
          }
        } else if (strcmp(tok.Literal, "value") == 0) {
          if (l.NextToken(&l).Type == ASSIGN) {
            Token tok = l.NextToken(&l);
            if (fabs(atof(tok.Literal)) < (v->figures[i].scale - (v->figures[i].scale*2)/(v->figures[i].height/4))) {
              char str[v->textboxs[0].column_n];
              memset(str, ' ', sizeof(str));
              for (int x = v->figures[i].x; x < (v->figures[i].width + v->figures[i].x); x++) {
                str[x/2] = '-';
              }
              v->textboxs[0].update(&v->textboxs[0], str, fit_figure_y(atof(tok.Literal), &(v->figures[i]))/4);
            }
          }
        }
      }
    }
  }
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
#endif
