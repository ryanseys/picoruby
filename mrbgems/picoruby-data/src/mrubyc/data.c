#include <mrubyc.h>
#include <stdio.h>

typedef struct DataSubclass {
  mrbc_class *cls;
  mrbc_value member_keys;
} data_subclass_t;

typedef struct DataInstance {
  mrbc_value members;
} data_instance_t;

static mrbc_class *class_Data;

/* ---- destructors ---- */

static void
data_subclass_destructor(mrbc_value *self)
{
  data_subclass_t *data = (data_subclass_t *)self->instance->data;
  mrbc_decref(&data->member_keys);
}

static void
data_instance_destructor(mrbc_value *self)
{
  data_instance_t *data = (data_instance_t *)self->instance->data;
  mrbc_decref(&data->members);
}

/*
 * Instance methods
 */

static void
c_method_missing(mrbc_vm *vm, mrbc_value *v, int argc)
{
  mrbc_value key = GET_ARG(1);
  data_instance_t *instance_data = (data_instance_t *)v->instance->data;
  mrbc_value members = instance_data->members;
  if (!mrbc_hash_search(&members, &key)) {
    mrbc_raise(vm, MRBC_CLASS(NoMethodError), "no such member");
    return;
  }
  mrbc_value value = mrbc_hash_get(&members, &key);
  mrbc_incref(&value);
  SET_RETURN(value);
}

static void
c_instance_to_h(mrbc_vm *vm, mrbc_value *v, int argc)
{
  data_instance_t *instance_data = (data_instance_t *)v->instance->data;
  mrbc_value members = instance_data->members;
  SET_RETURN(members);
}

static void
c_instance_members(mrbc_vm *vm, mrbc_value *v, int argc)
{
  data_instance_t *instance_data = (data_instance_t *)v->instance->data;
  mrbc_value members = instance_data->members;
  int size = members.hash->n_stored / 2;
  mrbc_value keys = mrbc_array_new(vm, size);
  for (int i = 0; i < size; i++) {
    mrbc_value key = members.hash->data[i * 2];
    mrbc_array_set(&keys, i, &key);
  }
  SET_RETURN(keys);
}

static void
c_instance_with(mrbc_vm *vm, mrbc_value *v, int argc)
{
  data_instance_t *instance_data = (data_instance_t *)v->instance->data;
  mrbc_value members = instance_data->members;
  int member_count = members.hash->n_stored / 2;

  /* changes given as keywords (v[argc + 1]) or a positional Hash */
  mrbc_value changes = mrbc_nil_value();
  if (v[argc + 1].tt == MRBC_TT_HASH) {
    changes = v[argc + 1];
  } else if (argc >= 1 && GET_ARG(1).tt == MRBC_TT_HASH) {
    changes = GET_ARG(1);
  } else if (argc >= 1) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "expected a Hash");
    return;
  }
  if (changes.tt == MRBC_TT_HASH) {
    int csize = changes.hash->n_stored / 2;
    for (int i = 0; i < csize; i++) {
      mrbc_value ckey = changes.hash->data[i * 2];
      if (!mrbc_hash_search(&members, &ckey)) {
        mrbc_raise(vm, MRBC_CLASS(ArgumentError), "unknown keyword");
        return;
      }
    }
  }

  mrbc_value self = mrbc_instance_new(vm, v->instance->cls, sizeof(data_instance_t));
  data_instance_t *new_data = (data_instance_t *)self.instance->data;
  memset(new_data, 0, sizeof(data_instance_t));

  mrbc_value new_members = mrbc_hash_new(vm, member_count);
  for (int i = 0; i < member_count; i++) {
    mrbc_value key = members.hash->data[i * 2];
    mrbc_value val;
    if (changes.tt == MRBC_TT_HASH && mrbc_hash_search(&changes, &key)) {
      val = mrbc_hash_get(&changes, &key);
    } else {
      val = members.hash->data[i * 2 + 1];
    }
    mrbc_hash_set(&new_members, &key, &val);
    mrbc_incref(&val);
  }
  mrbc_incref(&new_members);
  new_data->members = new_members;
  SET_RETURN(self);
}

static void
c_instance_is_a_q(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (argc != 1) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments");
    return;
  }
  mrbc_value arg = GET_ARG(1);
  if (arg.tt == MRBC_TT_CLASS) {
    if (arg.cls == class_Data) {
      SET_TRUE_RETURN();
    } else {
      SET_FALSE_RETURN();
    }
  } else if (arg.tt == MRBC_TT_OBJECT) {
    data_subclass_t *subclass_data = (data_subclass_t *)arg.instance->data;
    if (subclass_data->cls == v->instance->cls) {
      SET_TRUE_RETURN();
    } else {
      SET_FALSE_RETURN();
    }
  } else {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "not a class");
  }
}

/*
 * Subclass Class methods
 */

static void
c_new(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (v->cls == class_Data) {
    mrbc_raise(vm, MRBC_CLASS(NoMethodError), "undefined method 'new' for class Data");
    return;
  }
  data_subclass_t *subclass_data = (data_subclass_t *)v->instance->data;
  int member_count = subclass_data->member_keys.array->data_size;

  /* keyword init: all args given as keywords (mrubyc passes the keyword hash
     at v[argc + 1], not counted in argc) */
  int kw = 0;
  mrbc_value arg0 = v[argc + 1];
  if (argc == 0 && arg0.tt == MRBC_TT_HASH &&
      (arg0.hash->n_stored / 2) == member_count) {
    kw = 1;
    for (int i = 0; i < member_count; i++) {
      mrbc_value key = mrbc_array_get(&subclass_data->member_keys, i);
      if (!mrbc_hash_search(&arg0, &key)) { kw = 0; break; }
    }
  }

  if (!kw && member_count != argc) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments");
    return;
  }
  mrbc_value self = mrbc_instance_new(vm, subclass_data->cls, sizeof(data_instance_t));
  data_instance_t *instance_data = (data_instance_t *)self.instance->data;
  memset(instance_data, 0, sizeof(data_instance_t));

  mrbc_value members = mrbc_hash_new(vm, member_count);
  for (int i = 0; i < member_count; i++) {
    mrbc_value key = mrbc_array_get(&subclass_data->member_keys, i);
    mrbc_value value = kw ? mrbc_hash_get(&arg0, &key) : GET_ARG(i + 1);
    mrbc_hash_set(&members, &key, &value);
    mrbc_incref(&value);
  }
  mrbc_incref(&members);
  instance_data->members = members;
  SET_RETURN(self);
}

static void
c_members(mrbc_vm *vm, mrbc_value *v, int argc)
{
  if (v->cls == class_Data) {
    mrbc_raise(vm, MRBC_CLASS(NoMethodError), "undefined method 'members' for class Data");
    return;
  }
  data_subclass_t *subclass_data = (data_subclass_t *)v->instance->data;
  mrbc_value member_keys = subclass_data->member_keys;
  SET_RETURN(member_keys);
}

/*
 * Data class Class methods
 */

static void
c_define(mrbc_vm *vm, mrbc_value *v, int argc)
{
  mrbc_value subclass = mrbc_instance_new(vm, v->cls, sizeof(data_subclass_t));
  data_subclass_t *data = (data_subclass_t *)subclass.instance->data;
  memset(data, 0, sizeof(data_subclass_t));

  char *class_name = mrbc_alloc(vm, 15);
  memset(class_name, 0, 15);
  sprintf(class_name, "%p", subclass.instance);
  class_name[14] = '\0';
  mrbc_class *cls = mrbc_define_class(vm, class_name, class_Data);
  mrbc_define_destructor(cls, data_instance_destructor);

  mrbc_value member_keys = mrbc_array_new(vm, argc);
  for (int i = 0; i < argc; i++) {
    mrbc_value arg = GET_ARG(i + 1);
    mrbc_value key;
    switch (arg.tt) {
      case MRBC_TT_STRING: {
        key = mrbc_symbol_value(mrbc_str_to_symid((const char *)arg.string->data));
        break;
        }
      case MRBC_TT_SYMBOL: {
        key = arg;
        break;
        }
      default: {
        mrbc_raise(vm, MRBC_CLASS(TypeError), "not a symbol nor a string");
        return;
      }
    }
    mrbc_array_set(&member_keys, i, &key);
  }
  mrbc_define_method(vm, cls, "method_missing", c_method_missing);
  mrbc_define_method(vm, cls, "members", c_instance_members);
  mrbc_define_method(vm, cls, "with", c_instance_with);
  mrbc_define_method(vm, cls, "to_h", c_instance_to_h);
  mrbc_define_method(vm, cls, "is_a?", c_instance_is_a_q);

  data->cls = cls;
  mrbc_incref(&member_keys);
  data->member_keys = member_keys;
  SET_RETURN(subclass);
}

void
mrbc_data_init(mrbc_vm *vm)
{
  class_Data = mrbc_define_class(vm, "Data", mrbc_class_object);
  mrbc_define_destructor(class_Data, data_subclass_destructor);

  mrbc_define_method(vm, class_Data, "define", c_define);
  mrbc_define_method(vm, class_Data, "new", c_new);
  mrbc_define_method(vm, class_Data, "members", c_members);
}
