#pragma once

#include <string.h>

enum TokenType {
  ILLEGAL,
  EndOfFile,
  IDENT,
  INT,
  ASSIGN,
  COMMA,
  LPAREN,
  RPAREN,
  SET,
};

struct Token {
  enum TokenType Type;
  const char *Literal;
  const char *TypeStr;
};

typedef struct Token Token;

Token newToken(enum TokenType Type, const char *Literal) {
  struct Token tok;
  tok.Type = Type;
  tok.Literal = Literal;
  switch (tok.Type) {
    case ILLEGAL:
      tok.TypeStr = "ILLEGAL";
      break;
    case EndOfFile:
      tok.TypeStr = "EndOfFile";
      break;
    case IDENT:
      tok.TypeStr = "IDENT";
      break;
    case INT:
      tok.TypeStr = "INT";
      break;
    case ASSIGN:
      tok.TypeStr = "ASSIGN";
      break;
    case COMMA:
      tok.TypeStr = "COMMA";
      break;
    case LPAREN:
      tok.TypeStr = "LPAREN";
      break;
    case RPAREN:
      tok.TypeStr = "RPAREN";
      break;
    case SET:
      tok.TypeStr = "SET";
      break;
  } 
  return tok;
}

enum TokenType LookupIdent(char *ident) {
  enum TokenType tok = IDENT;
  if ((strcmp(ident, "set") == 0)) {
    tok = SET;
  } else if (strcmp(ident, "hoge") == 0) {
  }
  return tok;
}
