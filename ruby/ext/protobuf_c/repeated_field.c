// Protocol Buffers - Google's data interchange format
// Copyright 2014 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "protobuf.h"

// -----------------------------------------------------------------------------
// Repeated field container type.
// -----------------------------------------------------------------------------

const rb_data_type_t RepeatedField_type = {
  "Google::Protobuf::RepeatedField",
  { RepeatedField_mark, RepeatedField_free, NULL },
};

VALUE cRepeatedField;

RepeatedField* ruby_to_RepeatedField(VALUE _self) {
  RepeatedField* self;
  TypedData_Get_Struct(_self, RepeatedField, &RepeatedField_type, self);
  return self;
}

// Shared methods across all types.
VALUE RepeatedField_each(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  upb_fieldtype_t field_type = self->field_type;
  VALUE field_type_class = self->field_type_class;
  int element_size = native_slot_size(field_type);

  size_t off = 0;
  for (int i = 0; i < self->size; i++, off += element_size) {
    void* memory = (void *) (((uint8_t *)self->elements) + off);
    VALUE val = native_slot_get(field_type, field_type_class, memory);
    rb_yield(val);
  }
  return Qnil;
}

VALUE RepeatedField_index(VALUE _self, VALUE _index) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  int element_size = native_slot_size(self->field_type);
  upb_fieldtype_t field_type = self->field_type;
  VALUE field_type_class = self->field_type_class;

  int index = NUM2INT(_index);
  if (index < 0 || index >= self->size) {
    rb_raise(rb_eRangeError, "Index out of range");
  }

  void* memory = (void *) (((uint8_t *)self->elements) + index * element_size);
  return native_slot_get(field_type, field_type_class, memory);
}

VALUE RepeatedField_index_set(VALUE _self, VALUE _index, VALUE val) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  upb_fieldtype_t field_type = self->field_type;
  VALUE field_type_class = self->field_type_class;
  int element_size = native_slot_size(field_type);

  int index = NUM2INT(_index);
  if (index < 0 || index >= self->size) {
    rb_raise(rb_eRangeError, "Index out of range");
  }

  void* memory = (void *) (((uint8_t *)self->elements) + index * element_size);
  native_slot_set(field_type, field_type_class, memory, val);
  return Qnil;
}

static int kInitialSize = 8;

void RepeatedField_reserve(RepeatedField* self, int new_size) {
  if (new_size <= self->capacity) {
    return;
  }
  if (self->capacity == 0) {
    self->capacity = kInitialSize;
  }
  while (self->capacity < new_size) {
    self->capacity *= 2;
  }
  void* old_elems = self->elements;
  int elem_size = native_slot_size(self->field_type);
  self->elements = ALLOC_N(uint8_t, elem_size * self->capacity);
  if (old_elems != NULL) {
    memcpy(self->elements, old_elems, self->size * elem_size);
    xfree(old_elems);
  }
}

VALUE RepeatedField_push(VALUE _self, VALUE val) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  upb_fieldtype_t field_type = self->field_type;
  int element_size = native_slot_size(field_type);
  RepeatedField_reserve(self, self->size + 1);
  int index = self->size;
  void* memory = (void *) (((uint8_t *)self->elements) + index * element_size);
  native_slot_set(field_type, self->field_type_class, memory, val);
  // native_slot_set may raise an error; bump index only after set.
  self->size++;
  return Qnil;
}

// Used by parsing handlers.
void RepeatedField_push_native(VALUE _self, void* data) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  upb_fieldtype_t field_type = self->field_type;
  int element_size = native_slot_size(field_type);
  RepeatedField_reserve(self, self->size + 1);
  int index = self->size;
  void* memory = (void *) (((uint8_t *)self->elements) + index * element_size);
  memcpy(memory, data, element_size);
  self->size++;
}

void* RepeatedField_index_native(VALUE _self, int index) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  upb_fieldtype_t field_type = self->field_type;
  int element_size = native_slot_size(field_type);
  return ((uint8_t *)self->elements) + index * element_size;
}

VALUE RepeatedField_pop(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  upb_fieldtype_t field_type = self->field_type;
  VALUE field_type_class = self->field_type_class;
  int element_size = native_slot_size(field_type);
  if (self->size == 0) {
    rb_raise(rb_eRangeError, "Pop from empty repeated field is not allowed.");
  }
  int index = self->size - 1;
  void* memory = (void *) (((uint8_t *)self->elements) + index * element_size);
  VALUE ret = native_slot_get(field_type, field_type_class, memory);
  self->size--;
  return ret;
}

VALUE RepeatedField_insert(int argc, VALUE* argv, VALUE _self) {
  for (int i = 0; i < argc; i++) {
    RepeatedField_push(_self, argv[i]);
  }
  return Qnil;
}

VALUE RepeatedField_replace(VALUE _self, VALUE list) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  Check_Type(list, T_ARRAY);
  self->size = 0;
  for (int i = 0; i < RARRAY_LEN(list); i++) {
    RepeatedField_push(_self, rb_ary_entry(list, i));
  }
  return Qnil;
}

VALUE RepeatedField_clear(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  self->size = 0;
  return Qnil;
}

VALUE RepeatedField_length(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  return INT2NUM(self->size);
}

static VALUE RepeatedField_new_this_type(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  VALUE new_rptfield = Qnil;
  VALUE element_type = fieldtype_to_ruby(self->field_type);
  if (self->field_type_class != Qnil) {
    new_rptfield = rb_funcall(CLASS_OF(_self), rb_intern("new"), 2,
                              element_type, self->field_type_class);
  } else {
    new_rptfield = rb_funcall(CLASS_OF(_self), rb_intern("new"), 1,
                              element_type);
  }
  return new_rptfield;
}

VALUE RepeatedField_dup(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  VALUE new_rptfield = RepeatedField_new_this_type(_self);
  RepeatedField* new_rptfield_self = ruby_to_RepeatedField(new_rptfield);
  RepeatedField_reserve(new_rptfield_self, self->size);
  upb_fieldtype_t field_type = self->field_type;
  size_t elem_size = native_slot_size(field_type);
  size_t off = 0;
  for (int i = 0; i < self->size; i++, off += elem_size) {
    void* to_mem = (uint8_t *)new_rptfield_self->elements + off;
    void* from_mem = (uint8_t *)self->elements + off;
    native_slot_dup(field_type, to_mem, from_mem);
    new_rptfield_self->size++;
  }

  return new_rptfield;
}

VALUE RepeatedField_shallow_dup(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  VALUE new_rptfield = RepeatedField_new_this_type(_self);
  RepeatedField* new_rptfield_self = ruby_to_RepeatedField(new_rptfield);
  RepeatedField_reserve(new_rptfield_self, self->size);
  upb_fieldtype_t field_type = self->field_type;
  size_t elem_size = native_slot_size(field_type);
  size_t off = 0;
  for (int i = 0; i < self->size; i++, off += elem_size) {
    void* to_mem = (uint8_t *)new_rptfield_self->elements + off;
    void* from_mem = (uint8_t *)self->elements + off;
    memcpy(to_mem, from_mem, native_slot_size(field_type));
    new_rptfield_self->size++;
  }

  return new_rptfield;
}

VALUE RepeatedField_eq(VALUE _self, VALUE _other) {
  if (_self == _other) {
    return Qtrue;
  }
  RepeatedField* self = ruby_to_RepeatedField(_self);

  // Inefficient but workable: to support comparison to a generic array, we
  // build a temporary RepeatedField of our type.
  if (TYPE(_other) == T_ARRAY) {
    VALUE new_rptfield = RepeatedField_new_this_type(_self);
    for (int i = 0; i < RARRAY_LEN(_other); i++) {
      VALUE elem = rb_ary_entry(_other, i);
      RepeatedField_push(new_rptfield, elem);
    }
    _other = new_rptfield;
  }

  RepeatedField* other = ruby_to_RepeatedField(_other);
  if (self->field_type != other->field_type ||
      self->field_type_class != other->field_type_class ||
      self->size != other->size) {
    return Qfalse;
  }

  upb_fieldtype_t field_type = self->field_type;
  size_t elem_size = native_slot_size(field_type);
  size_t off = 0;
  for (int i = 0; i < self->size; i++, off += elem_size) {
    void* self_mem = ((uint8_t *)self->elements) + off;
    void* other_mem = ((uint8_t *)other->elements) + off;
    if (!native_slot_eq(field_type, self_mem, other_mem)) {
      return Qfalse;
    }
  }
  return Qtrue;
}

VALUE RepeatedField_hash(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);

  VALUE hash = LL2NUM(0);

  upb_fieldtype_t field_type = self->field_type;
  VALUE field_type_class = self->field_type_class;
  size_t elem_size = native_slot_size(field_type);
  size_t off = 0;
  for (int i = 0; i < self->size; i++, off += elem_size) {
    void* mem = ((uint8_t *)self->elements) + off;
    VALUE elem = native_slot_get(field_type, field_type_class, mem);
    hash = rb_funcall(hash, rb_intern("<<"), 1, INT2NUM(2));
    hash = rb_funcall(hash, rb_intern("^"), 1,
                      rb_funcall(elem, rb_intern("hash"), 0));
  }

  return hash;
}

VALUE RepeatedField_inspect(VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);

  VALUE str = rb_str_new2("[");

  bool first = true;

  upb_fieldtype_t field_type = self->field_type;
  VALUE field_type_class = self->field_type_class;
  size_t elem_size = native_slot_size(field_type);
  size_t off = 0;
  for (int i = 0; i < self->size; i++, off += elem_size) {
    void* mem = ((uint8_t *)self->elements) + off;
    VALUE elem = native_slot_get(field_type, field_type_class, mem);
    if (!first) {
      str = rb_str_cat2(str, ", ");
    } else {
      first = false;
    }
    str = rb_str_append(str, rb_funcall(elem, rb_intern("inspect"), 0));
  }

  str = rb_str_cat2(str, "]");
  return str;
}

VALUE RepeatedField_plus(VALUE _self, VALUE list) {
  VALUE dupped = RepeatedField_shallow_dup(_self);

  if (TYPE(list) == T_ARRAY) {
    for (int i = 0; i < RARRAY_LEN(list); i++) {
      VALUE elem = rb_ary_entry(list, i);
      RepeatedField_push(dupped, elem);
    }
  } else if (RB_TYPE_P(list, T_DATA) && RTYPEDDATA_P(list) &&
             RTYPEDDATA_TYPE(list) == &RepeatedField_type) {
    RepeatedField* self = ruby_to_RepeatedField(_self);
    RepeatedField* list_rptfield = ruby_to_RepeatedField(list);
    if (self->field_type != list_rptfield->field_type ||
        self->field_type_class != list_rptfield->field_type_class) {
      rb_raise(rb_eArgError,
               "Attempt to append RepeatedField with different element type.");
    }
    for (int i = 0; i < list_rptfield->size; i++) {
      void* mem = RepeatedField_index_native(list, i);
      RepeatedField_push_native(dupped, mem);
    }
  } else {
    rb_raise(rb_eArgError, "Unknown type appending to RepeatedField");
  }

  return dupped;
}

static void validate_type_class(upb_fieldtype_t type, VALUE klass) {
  if (rb_iv_get(klass, kDescriptorInstanceVar) == Qnil) {
    rb_raise(rb_eArgError,
             "Type class has no descriptor. Please pass a "
             "class or enum as returned by the DescriptorPool.");
  }
  if (type == UPB_TYPE_MESSAGE) {
    VALUE desc = rb_iv_get(klass, kDescriptorInstanceVar);
    if (!RB_TYPE_P(desc, T_DATA) || !RTYPEDDATA_P(desc) ||
        RTYPEDDATA_TYPE(desc) != &_Descriptor_type) {
      rb_raise(rb_eArgError, "Descriptor has an incorrect type.");
    }
    if (rb_get_alloc_func(klass) != &Message_alloc) {
      rb_raise(rb_eArgError,
               "Message class was not returned by the DescriptorPool.");
    }
  } else if (type == UPB_TYPE_ENUM) {
    VALUE enumdesc = rb_iv_get(klass, kDescriptorInstanceVar);
    if (!RB_TYPE_P(enumdesc, T_DATA) || !RTYPEDDATA_P(enumdesc) ||
        RTYPEDDATA_TYPE(enumdesc) != &_EnumDescriptor_type) {
      rb_raise(rb_eArgError, "Descriptor has an incorrect type.");
    }
  }
}

void RepeatedField_init_args(int argc, VALUE* argv,
                             VALUE _self) {
  RepeatedField* self = ruby_to_RepeatedField(_self);
  VALUE ary = Qnil;
  if (argc < 1) {
    rb_raise(rb_eArgError, "Expected at least 1 argument.");
  }
  self->field_type = ruby_to_fieldtype(argv[0]);

  if (self->field_type == UPB_TYPE_MESSAGE ||
      self->field_type == UPB_TYPE_ENUM) {
    if (argc < 2) {
      rb_raise(rb_eArgError, "Expected at least 2 arguments for message/enum.");
    }
    self->field_type_class = argv[1];
    if (argc > 2) {
      ary = argv[2];
    }
    validate_type_class(self->field_type, self->field_type_class);
  } else {
    if (argc > 2) {
      rb_raise(rb_eArgError, "Too many arguments: expected 1 or 2.");
    }
    if (argc > 1) {
      ary = argv[1];
    }
  }

  if (ary != Qnil) {
    if (!RB_TYPE_P(ary, T_ARRAY)) {
      rb_raise(rb_eArgError, "Expected array as initialize argument");
    }
    for (int i = 0; i < RARRAY_LEN(ary); i++) {
      RepeatedField_push(_self, rb_ary_entry(ary, i));
    }
  }
}

// Mark, free, alloc, init and class setup functions.

void RepeatedField_mark(void* _self) {
  RepeatedField* self = (RepeatedField*)_self;
  rb_gc_mark(self->field_type_class);
  upb_fieldtype_t field_type = self->field_type;
  int element_size = native_slot_size(field_type);
  for (int i = 0; i < self->size; i++) {
    void* memory = (((uint8_t *)self->elements) + i * element_size);
    native_slot_mark(self->field_type, memory);
  }
}

void RepeatedField_free(void* _self) {
  RepeatedField* self = (RepeatedField*)_self;
  xfree(self->elements);
  xfree(self);
}

VALUE RepeatedField_alloc(VALUE klass) {
  RepeatedField* self = ALLOC(RepeatedField);
  self->elements = NULL;
  self->size = 0;
  self->capacity = 0;
  self->field_type = -1;
  self->field_type_class = Qnil;
  VALUE ret = TypedData_Wrap_Struct(klass, &RepeatedField_type, self);
  return ret;
}

VALUE RepeatedField_init(int argc, VALUE* argv, VALUE self) {
  RepeatedField_init_args(argc, argv, self);
  return Qnil;
}

void RepeatedField_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "RepeatedField", rb_cObject);
  rb_define_alloc_func(klass, RepeatedField_alloc);
  cRepeatedField = klass;
  rb_gc_register_address(&cRepeatedField);

  rb_define_method(klass, "initialize",
                   RepeatedField_init, -1);
  rb_define_method(klass, "each", RepeatedField_each, 0);
  rb_define_method(klass, "[]", RepeatedField_index, 1);
  rb_define_method(klass, "[]=", RepeatedField_index_set, 2);
  rb_define_method(klass, "push", RepeatedField_push, 1);
  rb_define_method(klass, "pop", RepeatedField_pop, 0);
  rb_define_method(klass, "insert", RepeatedField_insert, -1);
  rb_define_method(klass, "replace", RepeatedField_replace, 1);
  rb_define_method(klass, "clear", RepeatedField_clear, 0);
  rb_define_method(klass, "length", RepeatedField_length, 0);
  rb_define_method(klass, "dup", RepeatedField_dup, 0);
  rb_define_method(klass, "==", RepeatedField_eq, 1);
  rb_define_method(klass, "hash", RepeatedField_hash, 0);
  rb_define_method(klass, "inspect", RepeatedField_inspect, 0);
  rb_define_method(klass, "+", RepeatedField_plus, 1);
  rb_include_module(klass, rb_mEnumerable);
}
