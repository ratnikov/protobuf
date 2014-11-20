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

#include <math.h>

// -----------------------------------------------------------------------------
// Ruby <-> native slot management.
// -----------------------------------------------------------------------------

size_t native_slot_size(upb_fieldtype_t type) {
  switch (type) {
    case UPB_TYPE_FLOAT:   return 4;
    case UPB_TYPE_DOUBLE:  return 8;
    case UPB_TYPE_BOOL:    return 1;
    case UPB_TYPE_STRING:  return sizeof(VALUE);
    case UPB_TYPE_BYTES:   return sizeof(VALUE);
    case UPB_TYPE_MESSAGE: return sizeof(VALUE);
    case UPB_TYPE_ENUM:    return 4;
    case UPB_TYPE_INT32:   return 4;
    case UPB_TYPE_INT64:   return 8;
    case UPB_TYPE_UINT32:  return 4;
    case UPB_TYPE_UINT64:  return 8;
    default: return 0;
  }
}

static void check_int_range_precision(upb_fieldtype_t type, VALUE val) {
  // NUM2{INT,UINT,LL,ULL} macros do the appropriate range checks on upper
  // bound; we just need to do precision checks (i.e., disallow rounding) and
  // check for < 0 on unsigned types.
  if (TYPE(val) == T_FLOAT) {
    double dbl_val = NUM2DBL(val);
    if (floor(dbl_val) != dbl_val) {
      rb_raise(rb_eRangeError,
               "Non-integral floating point value assigned to integer field.");
    }
  }
  if (type == UPB_TYPE_UINT32 || type == UPB_TYPE_UINT64) {
    if (rb_funcall(val, rb_intern("<"), 1, INT2NUM(0)) == Qtrue) {
      rb_raise(rb_eRangeError,
               "Assigning negative value to unsigned integer field.");
    }
  }
}

void native_slot_set(upb_fieldtype_t type, VALUE type_class,
                     void* memory, VALUE value) {
  switch (type) {
    case UPB_TYPE_FLOAT:
      if (TYPE(value) != T_FLOAT &&
          TYPE(value) != T_FIXNUM &&
          TYPE(value) != T_BIGNUM) {
        rb_raise(rb_eTypeError, "Expected number type for float field.");
      }
      *((float *)memory) = NUM2DBL(value);
      break;
    case UPB_TYPE_DOUBLE:
      if (TYPE(value) != T_FLOAT &&
          TYPE(value) != T_FIXNUM &&
          TYPE(value) != T_BIGNUM) {
        rb_raise(rb_eTypeError, "Expected number type for double field.");
      }
      *((double *)memory) = NUM2DBL(value);
      break;
    case UPB_TYPE_BOOL: {
      int8_t val = -1;
      if (value == Qtrue) {
        val = 1;
      } else if (value == Qfalse) {
        val = 0;
      } else {
        rb_raise(rb_eTypeError, "Invalid argument for boolean field.");
      }
      *((int8_t *)memory) = val;
      break;
    }
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES: {
      if (CLASS_OF(value) != rb_cString) {
        rb_raise(rb_eTypeError, "Invalid argument for string field.");
      }
      // TODO(cfallin): convert encoding to UTF-8 for STRING and US-ASCII-8 for
      // BYTES.
      *((VALUE *)memory) = value;
      break;
    }
    case UPB_TYPE_MESSAGE: {
      if (CLASS_OF(value) != type_class) {
        rb_raise(rb_eTypeError,
                 "Invalid type %s to assign to submessage field.",
                 rb_class2name(CLASS_OF(value)));
      }
      *((VALUE *)memory) = value;
      break;
    }
    case UPB_TYPE_ENUM: {
      if (TYPE(value) != T_FIXNUM &&
          TYPE(value) != T_BIGNUM &&
          TYPE(value) != T_SYMBOL) {
        rb_raise(rb_eTypeError,
                 "Expected number or symbol type for enum field.");
      }
      int32_t int_val = 0;
      if (TYPE(value) == T_SYMBOL) {
        // Ensure that the given symbol exists in the enum module.
        VALUE lookup = rb_const_get(type_class, SYM2ID(value));
        if (lookup == Qnil) {
          rb_raise(rb_eRangeError, "Unknown symbol value for enum field.");
        } else {
          int_val = NUM2INT(lookup);
        }
      } else {
        check_int_range_precision(UPB_TYPE_INT32, value);
        int_val = NUM2INT(value);
      }
      *((int32_t *)memory) = int_val;
      break;
    }
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_UINT64:
      if (TYPE(value) != T_FLOAT &&
          TYPE(value) != T_FIXNUM &&
          TYPE(value) != T_BIGNUM) {
        rb_raise(rb_eTypeError, "Expected number type for integral field.");
      }
      check_int_range_precision(type, value);
      switch (type) {
        case UPB_TYPE_INT32:
        *((int32_t *)memory) = NUM2INT(value);
        break;
      case UPB_TYPE_INT64:
        *((int64_t *)memory) = NUM2LL(value);
        break;
      case UPB_TYPE_UINT32:
        *((uint32_t *)memory) = NUM2UINT(value);
        break;
      case UPB_TYPE_UINT64:
        *((uint64_t *)memory) = NUM2ULL(value);
        break;
      default:
        break;
      }
      break;
    default:
      break;
  }
}

VALUE native_slot_get(upb_fieldtype_t type, VALUE type_class, void* memory) {
  switch (type) {
    case UPB_TYPE_FLOAT:
      return DBL2NUM(*((float *)memory));
    case UPB_TYPE_DOUBLE:
      return DBL2NUM(*((double *)memory));
    case UPB_TYPE_BOOL:
      return *((int8_t *)memory) ? Qtrue : Qfalse;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
    case UPB_TYPE_MESSAGE:
      return *((VALUE *)memory);
    case UPB_TYPE_ENUM: {
      int32_t val = *((int32_t *)memory);
      VALUE symbol = enum_lookup(type_class, INT2NUM(val));
      if (symbol == Qnil) {
        return INT2NUM(val);
      } else {
        return symbol;
      }
    }
    case UPB_TYPE_INT32:
      return INT2NUM(*((int32_t *)memory));
    case UPB_TYPE_INT64:
      return LL2NUM(*((int64_t *)memory));
    case UPB_TYPE_UINT32:
      return UINT2NUM(*((uint32_t *)memory));
    case UPB_TYPE_UINT64:
      return ULL2NUM(*((uint64_t *)memory));
    default:
      return Qnil;
  }
}

void native_slot_init(upb_fieldtype_t type, void* memory) {
  switch (type) {
    case UPB_TYPE_FLOAT:
      *((float *)memory) = 0.0;
      break;
    case UPB_TYPE_DOUBLE:
      *((double *)memory) = 0.0;
      break;
    case UPB_TYPE_BOOL:
      *((int8_t *)memory) = 0;
      break;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      // TODO(cfallin): set encoding appropriately
      *((VALUE *)memory) = rb_str_new2("");
      break;
    case UPB_TYPE_MESSAGE:
      *((VALUE *)memory) = Qnil;
      break;
    case UPB_TYPE_ENUM:
    case UPB_TYPE_INT32:
      *((int32_t *)memory) = 0;
      break;
    case UPB_TYPE_INT64:
      *((int64_t *)memory) = 0;
      break;
    case UPB_TYPE_UINT32:
      *((uint32_t *)memory) = 0;
      break;
    case UPB_TYPE_UINT64:
      *((uint64_t *)memory) = 0;
      break;
    default:
      break;
  }
}

void native_slot_mark(upb_fieldtype_t type, void* memory) {
  switch (type) {
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
    case UPB_TYPE_MESSAGE:
      rb_gc_mark(*(VALUE *)memory);
      break;
    default:
      break;
  }
}

void native_slot_dup(upb_fieldtype_t type, void* to, void* from) {
  switch (type) {
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
    case UPB_TYPE_MESSAGE: {
      VALUE from_val = *((VALUE *)from);
      *((VALUE *)to) = (from_val != Qnil) ?
          rb_funcall(from_val, rb_intern("dup"), 0) : Qnil;
      break;
    }
    default:
      memcpy(to, from, native_slot_size(type));
  }
}

bool native_slot_eq(upb_fieldtype_t type, void* mem1, void* mem2) {
  switch (type) {
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
    case UPB_TYPE_MESSAGE: {
      VALUE val1 = *((VALUE *)mem1);
      VALUE val2 = *((VALUE *)mem2);
      VALUE ret = rb_funcall(val1, rb_intern("=="), 1, val2);
      return ret == Qtrue;
    }
    default:
      return !memcmp(mem1, mem2, native_slot_size(type));
  }
}

// -----------------------------------------------------------------------------
// Memory layout management.
// -----------------------------------------------------------------------------

MessageLayout* create_layout(const upb_msgdef* msgdef) {
  MessageLayout* layout = ALLOC(MessageLayout);
  int nfields = upb_msgdef_numfields(msgdef);
  layout->offsets = ALLOC_N(size_t, nfields);

  upb_msg_iter it;
  size_t off = 0;
  for (upb_msg_begin(&it, msgdef); !upb_msg_done(&it); upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    size_t field_size =
        (upb_fielddef_label(field) == UPB_LABEL_REPEATED) ?
        sizeof(VALUE) : native_slot_size(upb_fielddef_type(field));
    // align current offset
    off = (off + field_size - 1) & ~(field_size - 1);
    layout->offsets[upb_fielddef_index(field)] = off;
    off += field_size;
  }

  layout->size = off;

  layout->msgdef = msgdef;
  upb_msgdef_ref(layout->msgdef, &layout->msgdef);

  return layout;
}

void free_layout(MessageLayout* layout) {
  xfree(layout->offsets);
  upb_msgdef_unref(layout->msgdef, &layout->msgdef);
  xfree(layout);
}

VALUE layout_get(MessageLayout* layout,
                 void* storage,
                 const upb_fielddef* field) {
  void* memory = ((uint8_t *)storage) +
      layout->offsets[upb_fielddef_index(field)];
  if (upb_fielddef_label(field) == UPB_LABEL_REPEATED) {
    return *((VALUE *)memory);
  } else {
    VALUE type_class = Qnil;
    if (upb_fielddef_type(field) == UPB_TYPE_MESSAGE) {
      Descriptor* submsgdesc =
          ruby_to_Descriptor(get_def_obj((void *)upb_fielddef_subdef(field)));
      type_class = submsgdesc->klass;
    } else if (upb_fielddef_type(field) == UPB_TYPE_ENUM) {
      EnumDescriptor* subenumdesc =
          ruby_to_EnumDescriptor(get_def_obj((void *)upb_fielddef_subdef(field)));
      type_class = subenumdesc->module;
    }
    return native_slot_get(upb_fielddef_type(field),
                           type_class,
                           memory);
  }
}

static void check_repeated_field_type(VALUE val, const upb_fielddef* field) {
  assert(upb_fielddef_label(field) == UPB_LABEL_REPEATED);

  if (!RB_TYPE_P(val, T_DATA) || !RTYPEDDATA_P(val) ||
      RTYPEDDATA_TYPE(val) != &RepeatedField_type) {
    rb_raise(rb_eTypeError, "Expected repeated field array");
  }

  RepeatedField* self = ruby_to_RepeatedField(val);
  if (self->field_type != upb_fielddef_type(field)) {
    rb_raise(rb_eTypeError, "Repeated field array has wrong element type");
  }

  if (upb_fielddef_type(field) == UPB_TYPE_MESSAGE ||
      upb_fielddef_type(field) == UPB_TYPE_ENUM) {
    RepeatedField* self = ruby_to_RepeatedField(val);
    if (self->field_type_class !=
        get_def_obj((void *)upb_fielddef_subdef(field))) {
      rb_raise(rb_eTypeError,
               "Repeated field array has wrong message/enum class");
    }
  }
}

void layout_set(MessageLayout* layout,
                void* storage,
                const upb_fielddef* field,
                VALUE val) {
  void* memory = ((uint8_t *)storage) +
      layout->offsets[upb_fielddef_index(field)];
  if (upb_fielddef_label(field) == UPB_LABEL_REPEATED) {
    check_repeated_field_type(val, field);
    *((VALUE *)memory) = val;
  } else {
    VALUE type_class = Qnil;
    if (upb_fielddef_type(field) == UPB_TYPE_MESSAGE) {
      Descriptor* submsgdesc =
          ruby_to_Descriptor(get_def_obj((void *)upb_fielddef_subdef(field)));
      type_class = submsgdesc->klass;
    } else if (upb_fielddef_type(field) == UPB_TYPE_ENUM) {
      EnumDescriptor* subenumdesc =
          ruby_to_EnumDescriptor(get_def_obj((void *)upb_fielddef_subdef(field)));
      type_class = subenumdesc->module;
    }
    native_slot_set(upb_fielddef_type(field), type_class, memory, val);
  }
}

void layout_init(MessageLayout* layout,
                 void* storage) {
  upb_msg_iter it;
  for (upb_msg_begin(&it, layout->msgdef);
       !upb_msg_done(&it);
       upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    void* memory = ((uint8_t *)storage) +
        layout->offsets[upb_fielddef_index(field)];

    if (upb_fielddef_label(field) == UPB_LABEL_REPEATED) {
      VALUE ary = Qnil;
      if (upb_fielddef_type(field) == UPB_TYPE_MESSAGE) {
        Descriptor* submsgdesc =
            ruby_to_Descriptor(get_def_obj((void *)upb_fielddef_subdef(field)));
        VALUE type_class = submsgdesc->klass;
        VALUE args[2] = { ID2SYM(rb_intern("message")), type_class };
        ary = rb_class_new_instance(2, args, cRepeatedField);
      } else if (upb_fielddef_type(field) == UPB_TYPE_ENUM) {
        EnumDescriptor* subenumdesc =
            ruby_to_EnumDescriptor(get_def_obj((void *)upb_fielddef_subdef(field)));
        VALUE type_class = subenumdesc->module;
        VALUE args[2] = { ID2SYM(rb_intern("enum")), type_class };
        ary = rb_class_new_instance(2, args, cRepeatedField);
      } else {
        VALUE args[1] = { fieldtype_to_ruby(upb_fielddef_type(field)) };
        ary = rb_class_new_instance(1, args, cRepeatedField);
      }
      *((VALUE *)memory) = ary;
    } else {
      native_slot_init(upb_fielddef_type(field), memory);
    }
  }
}

void layout_mark(MessageLayout* layout, void* storage) {
  upb_msg_iter it;
  for (upb_msg_begin(&it, layout->msgdef);
       !upb_msg_done(&it);
       upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    void* memory = ((uint8_t *)storage) +
        layout->offsets[upb_fielddef_index(field)];

    if (upb_fielddef_label(field) == UPB_LABEL_REPEATED) {
      rb_gc_mark(*((VALUE *)memory));
    } else {
      native_slot_mark(upb_fielddef_type(field), memory);
    }
  }
}

void layout_dup(MessageLayout* layout, void* to, void* from) {
  upb_msg_iter it;
  for (upb_msg_begin(&it, layout->msgdef);
       !upb_msg_done(&it);
       upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    void* to_memory = ((uint8_t *)to) +
        layout->offsets[upb_fielddef_index(field)];
    void* from_memory = ((uint8_t *)from) +
        layout->offsets[upb_fielddef_index(field)];

    if (upb_fielddef_label(field) == UPB_LABEL_REPEATED) {
      *((VALUE *)to_memory) = RepeatedField_dup(*((VALUE *)from_memory));
    } else {
      native_slot_dup(upb_fielddef_type(field), to_memory, from_memory);
    }
  }
}

VALUE layout_eq(MessageLayout* layout, void* msg1, void* msg2) {
  upb_msg_iter it;
  for (upb_msg_begin(&it, layout->msgdef);
       !upb_msg_done(&it);
       upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    void* msg1_memory = ((uint8_t *)msg1) +
        layout->offsets[upb_fielddef_index(field)];
    void* msg2_memory = ((uint8_t *)msg2) +
        layout->offsets[upb_fielddef_index(field)];

    if (upb_fielddef_label(field) == UPB_LABEL_REPEATED) {
      if (RepeatedField_eq(*((VALUE *)msg1_memory),
                           *((VALUE *)msg2_memory)) == Qfalse) {
        return Qfalse;
      }
    } else {
      if (!native_slot_eq(upb_fielddef_type(field),
                          msg1_memory, msg2_memory)) {
        return Qfalse;
      }
    }
  }
  return Qtrue;
}

VALUE layout_hash(MessageLayout* layout, void* storage) {
  VALUE hash = LL2NUM(0);

  upb_msg_iter it;
  for (upb_msg_begin(&it, layout->msgdef);
       !upb_msg_done(&it);
       upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    VALUE field_val = layout_get(layout, storage, field);

    // hash = (hash << 2) ^ field.hash();
    hash = rb_funcall(hash, rb_intern("<<"), 1, INT2NUM(2));
    hash = rb_funcall(hash, rb_intern("^"), 1,
                      rb_funcall(field_val, rb_intern("hash"), 0));
  }

  return hash;
}

VALUE layout_inspect(MessageLayout* layout, void* storage) {
  VALUE str = rb_str_new2("");

  upb_msg_iter it;
  bool first = true;
  for (upb_msg_begin(&it, layout->msgdef);
       !upb_msg_done(&it);
       upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    VALUE field_val = layout_get(layout, storage, field);

    if (!first) {
      str = rb_str_cat2(str, ", ");
    } else {
      first = false;
    }
    str = rb_str_cat2(str, upb_fielddef_name(field));
    str = rb_str_cat2(str, ": ");

    str = rb_str_append(str, rb_funcall(field_val, rb_intern("inspect"), 0));
  }

  return str;
}