#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <mruby/presym.h>
#include <mruby/object.h>
#include <mruby/proc.h>
#include <stdio.h>

typedef struct DataSubclass {
  struct RClass *cls;
} data_subclass_t;

typedef struct DataInstance {
  int dummy;
} data_instance_t;

static struct RClass *class_Data;

static const struct mrb_data_type data_subclass_type = {
  "DataSubclass", mrb_free
};

static const struct mrb_data_type data_instance_type = {
  "DataInstance", mrb_free
};

/*
 * Instance methods
 */

static mrb_value
mrb_data_method_missing(mrb_state *mrb, mrb_value self)
{
  mrb_sym method_name;
  mrb_value *args;
  mrb_int argc;

  mrb_get_args(mrb, "n*", &method_name, &args, &argc);

  mrb_value members = mrb_iv_get(mrb, self, MRB_IVSYM(members));
  mrb_value key = mrb_symbol_value(method_name);

  if (!mrb_hash_key_p(mrb, members, key)) {
    mrb_raisef(mrb, E_NOMETHOD_ERROR, "no such member: %n", method_name);
  }

  return mrb_hash_get(mrb, members, key);
}

static mrb_value
mrb_data_instance_to_h(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self, MRB_IVSYM(members));
}

static mrb_value
mrb_data_instance_members(mrb_state *mrb, mrb_value self)
{
  mrb_value members = mrb_iv_get(mrb, self, MRB_IVSYM(members));
  return mrb_hash_keys(mrb, members);
}

static mrb_value
mrb_data_instance_with(mrb_state *mrb, mrb_value self)
{
  mrb_value changes = mrb_nil_value();
  mrb_get_args(mrb, "|H", &changes);

  mrb_value members = mrb_iv_get(mrb, self, MRB_IVSYM(members));
  mrb_value new_members = mrb_hash_dup(mrb, members);
  if (!mrb_nil_p(changes)) {
    mrb_value keys = mrb_hash_keys(mrb, changes);
    for (mrb_int i = 0; i < RARRAY_LEN(keys); i++) {
      mrb_value key = mrb_ary_ref(mrb, keys, i);
      if (!mrb_hash_key_p(mrb, members, key)) {
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "unknown keyword: %v", key);
      }
    }
    mrb_hash_merge(mrb, new_members, changes);
  }

  data_instance_t *instance_data = (data_instance_t *)mrb_malloc(mrb, sizeof(data_instance_t));
  mrb_value instance = mrb_obj_value(Data_Wrap_Struct(mrb, mrb_obj_class(mrb, self), &data_instance_type, instance_data));
  mrb_iv_set(mrb, instance, MRB_IVSYM(members), new_members);
  return instance;
}

static mrb_value
mrb_data_instance_is_a(mrb_state *mrb, mrb_value self)
{
  mrb_value klass;
  mrb_get_args(mrb, "o", &klass);

  if (mrb_type(klass) == MRB_TT_CLASS) {
    struct RClass *c = mrb_class_ptr(klass);
    if (c == class_Data) {
      return mrb_true_value();
    }
    return mrb_false_value();
  } else {
    mrb_raise(mrb, E_TYPE_ERROR, "is_a? requires a Class");
  }
}

/*
 * Subclass Class methods
 */

static mrb_value
mrb_data_subclass_new(mrb_state *mrb, mrb_value self)
{
  if (mrb_type(self) == MRB_TT_CLASS) {
    struct RClass *c = mrb_class_ptr(self);
    if (c == class_Data) {
      mrb_raise(mrb, E_NOMETHOD_ERROR, "undefined method 'new' for class Data");
    }
  }

  data_subclass_t *subclass_data = (data_subclass_t *)DATA_PTR(self);
  if (!subclass_data) {
    mrb_raise(mrb, E_TYPE_ERROR, "uninitialized Data subclass");
  }

  mrb_value member_keys = mrb_iv_get(mrb, self, MRB_IVSYM(member_keys));
  mrb_int member_count = RARRAY_LEN(member_keys);

  mrb_value *args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  /* keyword init: a single Hash whose keys are exactly the members */
  mrb_bool kw = FALSE;
  if (argc == 1 && mrb_hash_p(args[0]) &&
      mrb_hash_size(mrb, args[0]) == member_count) {
    kw = TRUE;
    for (mrb_int i = 0; i < member_count; i++) {
      mrb_value key = mrb_ary_ref(mrb, member_keys, i);
      if (!mrb_hash_key_p(mrb, args[0], key)) { kw = FALSE; break; }
    }
  }

  if (!kw && argc != member_count) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "wrong number of arguments (given %d, expected %d)",
               argc, member_count);
  }

  data_instance_t *instance_data = (data_instance_t *)mrb_malloc(mrb, sizeof(data_instance_t));
  mrb_value instance = mrb_obj_value(Data_Wrap_Struct(mrb, subclass_data->cls, &data_instance_type, instance_data));

  mrb_value members = mrb_hash_new_capa(mrb, member_count);
  for (mrb_int i = 0; i < member_count; i++) {
    mrb_value key = mrb_ary_ref(mrb, member_keys, i);
    mrb_value value = kw ? mrb_hash_get(mrb, args[0], key) : args[i];
    mrb_hash_set(mrb, members, key, value);
  }

  mrb_iv_set(mrb, instance, MRB_IVSYM(members), members);

  return instance;
}

static mrb_value
mrb_data_subclass_members(mrb_state *mrb, mrb_value self)
{
  if (mrb_type(self) == MRB_TT_CLASS) {
    struct RClass *c = mrb_class_ptr(self);
    if (c == class_Data) {
      mrb_raise(mrb, E_NOMETHOD_ERROR, "undefined method 'members' for class Data");
    }
  }

  return mrb_iv_get(mrb, self, MRB_IVSYM(member_keys));
}

/*
 * Data class Class methods
 */

static mrb_value
mrb_data_define(mrb_state *mrb, mrb_value self)
{
  mrb_value *args;
  mrb_int argc;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "*&", &args, &argc, &block);

  mrb_value subclass = mrb_obj_new(mrb, class_Data, 0, NULL);
  data_subclass_t *data = (data_subclass_t *)mrb_malloc(mrb, sizeof(data_subclass_t));

  char class_name[32];
  snprintf(class_name, sizeof(class_name), "%p", (void *)mrb_obj_ptr(subclass));

  struct RClass *cls = mrb_define_class(mrb, class_name, class_Data);

  mrb_value member_keys = mrb_ary_new_capa(mrb, argc);
  for (mrb_int i = 0; i < argc; i++) {
    mrb_value arg = args[i];
    mrb_value key;

    switch (mrb_type(arg)) {
      case MRB_TT_STRING:
        key = mrb_symbol_value(mrb_intern_str(mrb, arg));
        break;
      case MRB_TT_SYMBOL:
        key = arg;
        break;
      default:
        mrb_raise(mrb, E_TYPE_ERROR, "not a symbol nor a string");
    }
    mrb_ary_push(mrb, member_keys, key);
  }

  data->cls = cls;

  DATA_PTR(subclass) = data;
  DATA_TYPE(subclass) = &data_subclass_type;
  mrb_iv_set(mrb, subclass, MRB_IVSYM(member_keys), member_keys);

  mrb_define_method_id(mrb, cls, MRB_SYM(method_missing), mrb_data_method_missing, MRB_ARGS_ANY());
  mrb_define_method_id(mrb, cls, MRB_SYM(members), mrb_data_instance_members, MRB_ARGS_NONE());
  mrb_define_method_id(mrb, cls, MRB_SYM(with), mrb_data_instance_with, MRB_ARGS_OPT(1));
  mrb_define_method_id(mrb, cls, MRB_SYM(to_h), mrb_data_instance_to_h, MRB_ARGS_NONE());
  mrb_define_method_id(mrb, cls, MRB_SYM_Q(is_a), mrb_data_instance_is_a, MRB_ARGS_REQ(1));

  struct RClass *singleton = mrb_singleton_class_ptr(mrb, subclass);
  mrb_define_method_id(mrb, singleton, MRB_SYM(new), mrb_data_subclass_new, MRB_ARGS_ANY());
  mrb_define_method_id(mrb, singleton, MRB_SYM(members), mrb_data_subclass_members, MRB_ARGS_NONE());

  /* block form: evaluate the block in the value class to add methods */
  if (!mrb_nil_p(block)) {
    mrb_value cls_val = mrb_obj_value(cls);
    mrb_yield_with_class(mrb, block, 0, NULL, cls_val, cls);
  }

  return subclass;
}

void
mrb_picoruby_data_gem_init(mrb_state *mrb)
{
  class_Data = mrb_define_class_id(mrb, MRB_SYM(Data), mrb->object_class);
  MRB_SET_INSTANCE_TT(class_Data, MRB_TT_CDATA);

  mrb_define_class_method_id(mrb, class_Data, MRB_SYM(define), mrb_data_define, MRB_ARGS_ANY());
}

void
mrb_picoruby_data_gem_final(mrb_state *mrb)
{
}
