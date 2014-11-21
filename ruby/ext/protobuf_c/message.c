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
// Class/module creation from msgdefs and enumdefs, respectively.
// -----------------------------------------------------------------------------

void* Message_data(void* msg) {
  return ((uint8_t *)msg) + sizeof(MessageHeader);
}

void Message_mark(void* _self) {
  MessageHeader* self = (MessageHeader *)_self;
  rb_gc_mark(self->descriptor_rb);
  layout_mark(self->descriptor->layout, Message_data(self));
}

void Message_free(void* self) {
  xfree(self);
}

rb_data_type_t Message_type = {
  "Message",
  { Message_mark, Message_free, NULL },
};

VALUE Message_alloc(VALUE klass) {
  VALUE descriptor = rb_iv_get(klass, kDescriptorInstanceVar);
  Descriptor* desc = ruby_to_Descriptor(descriptor);
  MessageHeader* msg = (MessageHeader*)ALLOC_N(
      uint8_t, sizeof(MessageHeader) + desc->layout->size);
  memset(Message_data(msg), 0, desc->layout->size);

  // We wrap first so that everything in the message object is GC-rooted in case
  // a collection happens during object creation in layout_init().
  VALUE ret = TypedData_Wrap_Struct(klass, &Message_type, msg);
  msg->descriptor_rb = descriptor;
  msg->descriptor = desc;
  rb_iv_set(ret, kDescriptorInstanceVar, descriptor);

  layout_init(desc->layout, Message_data(msg));

  return ret;
}

VALUE Message_method_missing(int argc, VALUE* argv, VALUE _self) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);
  if (argc < 1) {
    rb_raise(rb_eArgError, "Expected method name as first argument.");
  }
  VALUE method_name = argv[0];
  if (!SYMBOL_P(method_name)) {
    rb_raise(rb_eArgError, "Expected symbol as method name.");
  }
  VALUE method_str = rb_id2str(SYM2ID(method_name));
  char* name = RSTRING_PTR(method_str);
  size_t name_len = RSTRING_LEN(method_str);
  bool setter = false;

  // Setters have names that end in '='.
  if (name[name_len - 1] == '=') {
    setter = true;
    name_len--;
  }

  const upb_fielddef* f = upb_msgdef_ntof(self->descriptor->msgdef,
                                          name, name_len);

  if (f == NULL) {
    rb_raise(rb_eArgError, "Unknown field");
  }

  if (setter) {
    if (argc < 2) {
      rb_raise(rb_eArgError, "No value provided to setter.");
    }
    layout_set(self->descriptor->layout, Message_data(self), f, argv[1]);
    return Qnil;
  } else {
    return layout_get(self->descriptor->layout, Message_data(self), f);
  }
}

int Message_initialize_kwarg(VALUE key, VALUE val, VALUE _self) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);

  if (!SYMBOL_P(key)) {
    rb_raise(rb_eArgError,
             "Expected symbols as hash keys in initialization map.");
  }

  VALUE method_str = rb_id2str(SYM2ID(key));
  char* name = RSTRING_PTR(method_str);
  const upb_fielddef* f = upb_msgdef_ntofz(self->descriptor->msgdef, name);
  if (f == NULL) {
    rb_raise(rb_eArgError,
             "Unknown field name in initialization map entry.");
  }

  if (upb_fielddef_label(f) == UPB_LABEL_REPEATED) {
    if (TYPE(val) != T_ARRAY) {
      rb_raise(rb_eArgError,
               "Expected array as initializer value for repeated field.");
    }
    VALUE ary = layout_get(self->descriptor->layout, Message_data(self), f);
    for (int i = 0; i < RARRAY_LEN(val); i++) {
      RepeatedField_push(ary, rb_ary_entry(val, i));
    }
  } else {
    layout_set(self->descriptor->layout, Message_data(self), f, val);
  }
  return 0;
}

VALUE Message_initialize(int argc, VALUE* argv, VALUE _self) {
  if (argc == 0) {
    return Qnil;
  }
  if (argc != 1) {
    rb_raise(rb_eArgError, "Expected 0 or 1 arguments.");
  }
  VALUE hash_args = argv[0];
  if (TYPE(hash_args) != T_HASH) {
    rb_raise(rb_eArgError, "Expected hash arguments.");
  }

  rb_hash_foreach(hash_args, Message_initialize_kwarg, _self);
  return Qnil;
}

VALUE Message_dup(VALUE _self) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);

  VALUE new_msg = rb_class_new_instance(0, NULL, CLASS_OF(_self));
  MessageHeader* new_msg_self;
  TypedData_Get_Struct(new_msg, MessageHeader, &Message_type, new_msg_self);

  layout_dup(self->descriptor->layout,
             Message_data(new_msg_self),
             Message_data(self));

  return new_msg;
}

VALUE Message_eq(VALUE _self, VALUE _other) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);

  MessageHeader* other;
  TypedData_Get_Struct(_other, MessageHeader, &Message_type, other);

  if (self->descriptor != other->descriptor) {
    return Qfalse;
  }

  return layout_eq(self->descriptor->layout,
                   Message_data(self),
                   Message_data(other));
}

VALUE Message_hash(VALUE _self) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);

  return layout_hash(self->descriptor->layout, Message_data(self));
}

VALUE Message_inspect(VALUE _self) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);

  VALUE str = rb_str_new2("<");
  str = rb_str_append(str, rb_str_new2(rb_class2name(CLASS_OF(_self))));
  str = rb_str_cat2(str, ": ");
  str = rb_str_append(str, layout_inspect(
      self->descriptor->layout, Message_data(self)));
  str = rb_str_cat2(str, ">");
  return str;
}

VALUE Message_index(VALUE _self, VALUE field_name) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);
  Check_Type(field_name, T_STRING);
  const upb_fielddef* field =
      upb_msgdef_ntofz(self->descriptor->msgdef, RSTRING_PTR(field_name));
  if (field == NULL) {
    return Qnil;
  }
  return layout_get(self->descriptor->layout, Message_data(self), field);
}

VALUE Message_index_set(VALUE _self, VALUE field_name, VALUE value) {
  MessageHeader* self;
  TypedData_Get_Struct(_self, MessageHeader, &Message_type, self);
  Check_Type(field_name, T_STRING);
  const upb_fielddef* field =
      upb_msgdef_ntofz(self->descriptor->msgdef, RSTRING_PTR(field_name));
  if (field == NULL) {
    rb_raise(rb_eArgError, "Unknown field: %s", RSTRING_PTR(field_name));
  }
  layout_set(self->descriptor->layout, Message_data(self), field, value);
  return Qnil;
}

VALUE Message_descriptor(VALUE klass) {
  return rb_iv_get(klass, kDescriptorInstanceVar);
}

VALUE build_class_from_descriptor(Descriptor* desc) {
  if (desc->layout == NULL) {
    desc->layout = create_layout(desc->msgdef);
  }
  if (desc->fill_method == NULL) {
    desc->fill_method = new_fillmsg_decodermethod(desc, &desc->fill_method);
  }
  if (desc->serialize_handlers == NULL) {
    desc->serialize_handlers =
        upb_pb_encoder_newhandlers(desc->msgdef, &desc->serialize_handlers);
  }

  const char* name = upb_msgdef_fullname(desc->msgdef);
  if (name == NULL) {
    rb_raise(rb_eRuntimeError, "Descriptor does not have assigned name.");
  }

  VALUE klass = rb_define_class_id(
      // Docs say this parameter is ignored. User will assign return value to
      // their own toplevel constant class name.
      rb_intern("Message"),
      rb_cObject);
  rb_iv_set(klass, kDescriptorInstanceVar, get_def_obj(desc->msgdef));
  rb_define_alloc_func(klass, Message_alloc);
  rb_define_method(klass, "method_missing",
                   Message_method_missing, -1);
  rb_define_method(klass, "initialize", Message_initialize, -1);
  rb_define_method(klass, "dup", Message_dup, 0);
  rb_define_method(klass, "==", Message_eq, 1);
  rb_define_method(klass, "hash", Message_hash, 0);
  rb_define_method(klass, "inspect", Message_inspect, 0);
  rb_define_method(klass, "[]", Message_index, 1);
  rb_define_method(klass, "[]=", Message_index_set, 2);
  rb_define_singleton_method(klass, "decode", Message_decode, 1);
  rb_define_singleton_method(klass, "encode", Message_encode, 1);
  rb_define_singleton_method(klass, "descriptor", Message_descriptor, 0);
  return klass;
}

VALUE enum_lookup(VALUE self, VALUE number) {
  int32_t num = NUM2INT(number);
  VALUE desc = rb_iv_get(self, kDescriptorInstanceVar);
  EnumDescriptor* enumdesc = ruby_to_EnumDescriptor(desc);

  const char* name = upb_enumdef_iton(enumdesc->enumdef, num);
  if (name == NULL) {
    return Qnil;
  } else {
    return ID2SYM(rb_intern(name));
  }
}

VALUE enum_resolve(VALUE self, VALUE sym) {
  const char* name = rb_id2name(SYM2ID(sym));
  VALUE desc = rb_iv_get(self, kDescriptorInstanceVar);
  EnumDescriptor* enumdesc = ruby_to_EnumDescriptor(desc);

  int32_t num = 0;
  bool found = upb_enumdef_ntoi(enumdesc->enumdef, name, &num);
  if (!found) {
    return Qnil;
  } else {
    return INT2NUM(num);
  }
}

VALUE enum_descriptor(VALUE self) {
  return rb_iv_get(self, kDescriptorInstanceVar);
}

VALUE build_module_from_enumdesc(EnumDescriptor* enumdesc) {
  VALUE mod = rb_define_module_id(
      rb_intern(upb_enumdef_fullname(enumdesc->enumdef)));

  upb_enum_iter it;
  for (upb_enum_begin(&it, enumdesc->enumdef);
       !upb_enum_done(&it);
       upb_enum_next(&it)) {
    const char* name = upb_enum_iter_name(&it);
    int32_t value = upb_enum_iter_number(&it);
    if (name[0] < 'A' || name[0] > 'Z') {
      rb_raise(rb_eTypeError,
               "Enum value '%s' does not start with an uppercase letter "
               "as is required for Ruby constants.",
               name);
    }
    rb_define_const(mod, name, INT2NUM(value));
  }

  rb_define_singleton_method(mod, "lookup", enum_lookup, 1);
  rb_define_singleton_method(mod, "resolve", enum_resolve, 1);
  rb_define_singleton_method(mod, "descriptor", enum_descriptor, 0);
  rb_iv_set(mod, kDescriptorInstanceVar, get_def_obj(enumdesc->enumdef));

  return mod;
}
