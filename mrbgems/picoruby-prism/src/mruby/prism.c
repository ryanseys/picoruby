#include <mruby.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/variable.h>

#include "prism.h"

/*
 * This binding reuses the prism library already compiled and linked by
 * mruby-compiler (standard allocator), so it adds no second copy of libprism.
 */

/* Prism.parse(source) -> String (pretty-printed AST dump) */
static mrb_value
mrb_prism_parse(mrb_state *mrb, mrb_value self)
{
  char *src;
  mrb_int len;
  mrb_get_args(mrb, "s", &src, &len);

  pm_parser_t parser;
  pm_parser_init(&parser, (const uint8_t *)src, (size_t)len, NULL);
  pm_node_t *root = pm_parse(&parser);

  pm_buffer_t buffer = { 0 };
  pm_prettyprint(&buffer, &parser, root);
  mrb_value str = mrb_str_new(mrb, buffer.value, buffer.length);
  pm_buffer_free(&buffer);

  pm_node_destroy(&parser, root);
  pm_parser_free(&parser);
  return str;
}

/* Prism.errors(source) -> [{message:, line:, column:}, ...] (empty if valid) */
static mrb_value
mrb_prism_errors(mrb_state *mrb, mrb_value self)
{
  char *src;
  mrb_int len;
  mrb_get_args(mrb, "s", &src, &len);

  pm_parser_t parser;
  pm_parser_init(&parser, (const uint8_t *)src, (size_t)len, NULL);
  pm_node_t *root = pm_parse(&parser);

  mrb_value result = mrb_ary_new(mrb);
  mrb_sym k_message = mrb_intern_lit(mrb, "message");
  mrb_sym k_line = mrb_intern_lit(mrb, "line");
  mrb_sym k_column = mrb_intern_lit(mrb, "column");

  for (pm_list_node_t *n = parser.error_list.head; n != NULL; n = n->next) {
    pm_diagnostic_t *d = (pm_diagnostic_t *)n;
    pm_line_column_t lc =
      pm_newline_list_line_column(&parser.newline_list, d->location.start, parser.start_line);
    mrb_value h = mrb_hash_new_capa(mrb, 3);
    mrb_hash_set(mrb, h, mrb_symbol_value(k_message),
                 mrb_str_new_cstr(mrb, d->message ? d->message : ""));
    mrb_hash_set(mrb, h, mrb_symbol_value(k_line), mrb_fixnum_value((mrb_int)lc.line));
    mrb_hash_set(mrb, h, mrb_symbol_value(k_column), mrb_fixnum_value((mrb_int)lc.column));
    mrb_ary_push(mrb, result, h);
  }

  pm_node_destroy(&parser, root);
  pm_parser_free(&parser);
  return result;
}

typedef struct {
  mrb_state *mrb;
  mrb_value ary;
  const uint8_t *base;
} lex_data_t;

static void
prism_lex_callback(void *data, pm_parser_t *parser, pm_token_t *token)
{
  lex_data_t *d = (lex_data_t *)data;
  mrb_state *mrb = d->mrb;
  mrb_value entry = mrb_ary_new_capa(mrb, 3);
  const char *name = pm_token_type_name(token->type);
  mrb_ary_push(mrb, entry, mrb_symbol_value(mrb_intern_cstr(mrb, name)));
  mrb_ary_push(mrb, entry, mrb_fixnum_value((mrb_int)(token->start - d->base)));
  mrb_ary_push(mrb, entry, mrb_fixnum_value((mrb_int)(token->end - d->base)));
  mrb_ary_push(mrb, d->ary, entry);
}

/* Prism.lex(source) -> [[type_symbol, start_offset, end_offset], ...] */
static mrb_value
mrb_prism_lex(mrb_state *mrb, mrb_value self)
{
  char *src;
  mrb_int len;
  mrb_get_args(mrb, "s", &src, &len);

  pm_parser_t parser;
  pm_parser_init(&parser, (const uint8_t *)src, (size_t)len, NULL);

  mrb_value ary = mrb_ary_new(mrb);
  lex_data_t data = { mrb, ary, (const uint8_t *)src };
  pm_lex_callback_t cb = { &data, prism_lex_callback };
  parser.lex_callback = &cb;

  pm_node_t *root = pm_parse(&parser);
  pm_node_destroy(&parser, root);
  pm_parser_free(&parser);
  return ary;
}

void
mrb_picoruby_prism_gem_init(mrb_state *mrb)
{
  struct RClass *prism = mrb_define_class_id(mrb, mrb_intern_lit(mrb, "Prism"), mrb->object_class);
  mrb_define_class_method_id(mrb, prism, mrb_intern_lit(mrb, "parse"), mrb_prism_parse, MRB_ARGS_REQ(1));
  mrb_define_class_method_id(mrb, prism, mrb_intern_lit(mrb, "lex"), mrb_prism_lex, MRB_ARGS_REQ(1));
  mrb_define_class_method_id(mrb, prism, mrb_intern_lit(mrb, "errors"), mrb_prism_errors, MRB_ARGS_REQ(1));
}

void
mrb_picoruby_prism_gem_final(mrb_state *mrb)
{
}
