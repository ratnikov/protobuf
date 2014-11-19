#!/usr/bin/ruby

require 'protobuf'
require 'test/unit'

# ------------- generated code --------------

symtab = Google::Protobuf::SymbolTable.global_symtab
symtab.build do
  add_message "TestMessage" do
    optional :optional_int32,  :int32,        1
    optional :optional_int64,  :int64,        2
    optional :optional_uint32, :uint32,       3
    optional :optional_uint64, :uint64,       4
    optional :optional_bool,   :bool,         5
    optional :optional_float,  :float,        6
    optional :optional_double, :double,       7
    optional :optional_string, :string,       8
    optional :optional_bytes,  :bytes,        9
    optional :optional_msg,    :message,      10, "TestMessage2"
    optional :optional_enum,   :enum,         11, "TestEnum"

    repeated :repeated_int32,  :int32,        12
    repeated :repeated_int64,  :int64,        13
    repeated :repeated_uint32, :uint32,       14
    repeated :repeated_uint64, :uint64,       15
    repeated :repeated_bool,   :bool,         16
    repeated :repeated_float,  :float,        17
    repeated :repeated_double, :double,       18
    repeated :repeated_string, :string,       19
    repeated :repeated_bytes,  :bytes,        20
    repeated :repeated_msg,    :message,      21, "TestMessage2"
    repeated :repeated_enum,   :enum,         22, "TestEnum"
  end
  add_message "TestMessage2" do
    optional :foo, :int32, 1
  end
  add_message "Recursive1" do
    optional :foo, :message, 1, "Recursive2"
  end
  add_message "Recursive2" do
    optional :foo, :message, 1, "Recursive1"
  end
  add_enum "TestEnum" do
    value :Default, 0
    value :A, 1
    value :B, 2
    value :C, 3
  end
end

TestMessage = symtab.get_class("TestMessage")
TestMessage2 = symtab.get_class("TestMessage2")
Recursive1 = symtab.get_class("Recursive1")
Recursive2 = symtab.get_class("Recursive2")
TestEnum = symtab.get_enum("TestEnum")

# ------------ test cases ---------------

class MessageContainerTest < Test::Unit::TestCase

  def test_defaults
    m = TestMessage.new
    assert m.optional_int32 == 0
    assert m.optional_int64 == 0
    assert m.optional_uint32 == 0
    assert m.optional_uint64 == 0
    assert m.optional_bool == false
    assert m.optional_float == 0.0
    assert m.optional_double == 0.0
    assert m.optional_string == ""
    assert m.optional_bytes == ""
    assert m.optional_msg == nil
    assert m.optional_enum == :Default
  end

  def test_setters
    m = TestMessage.new
    m.optional_int32 = -42
    assert m.optional_int32 == -42
    m.optional_int64 = -0x1_0000_0000
    assert m.optional_int64 == -0x1_0000_0000
    m.optional_uint32 = 0x9000_0000
    assert m.optional_uint32 == 0x9000_0000
    m.optional_uint64 = 0x9000_0000_0000_0000
    assert m.optional_uint64 == 0x9000_0000_0000_0000
    m.optional_bool = true
    assert m.optional_bool == true
    m.optional_float = 0.5
    assert m.optional_float == 0.5
    m.optional_double = 0.5
    m.optional_string = "hello"
    assert m.optional_string == "hello"
    m.optional_bytes = "world"
    assert m.optional_bytes == "world"
    m.optional_msg = TestMessage2.new(:foo => 42)
    assert m.optional_msg == TestMessage2.new(:foo => 42)
  end

  def test_ctor_args
    m = TestMessage.new(:optional_int32 => -42,
                        :optional_msg => TestMessage2.new,
                        :optional_enum => :C,
                        :repeated_string => ["hello", "there", "world"])
    assert m.optional_int32 == -42
    assert m.optional_msg.class == TestMessage2
    assert m.repeated_string.length == 3
    assert m.optional_enum == :C
    assert m.repeated_string[0] == "hello"
    assert m.repeated_string[1] == "there"
    assert m.repeated_string[2] == "world"
  end

  def test_inspect
    m = TestMessage.new(:optional_int32 => -42,
                        :optional_enum => :A,
                        :optional_msg => TestMessage2.new,
                        :repeated_string => ["hello", "there", "world"])
    expected = '<TestMessage: optional_int32: -42, optional_int64: 0, optional_uint32: 0, optional_uint64: 0, optional_bool: false, optional_float: 0.0, optional_double: 0.0, optional_string: "", optional_bytes: "", optional_msg: <TestMessage2: foo: 0>, optional_enum: :A, repeated_int32: [], repeated_int64: [], repeated_uint32: [], repeated_uint64: [], repeated_bool: [], repeated_float: [], repeated_double: [], repeated_string: ["hello", "there", "world"], repeated_bytes: [], repeated_msg: [], repeated_enum: []>'
    assert m.inspect == expected
  end

  def test_type_errors
    m = TestMessage.new
    assert_raise TypeError do
      m.optional_int32 = "hello"
    end
    assert_raise TypeError do
      m.optional_string = 42
    end
    assert_raise TypeError do
      m.optional_string = nil
    end
    assert_raise TypeError do
      m.optional_bool = 42
    end
    assert_raise TypeError do
      m.optional_msg = TestMessage.new  # expects TestMessage2
    end

    assert_raise TypeError do
      m.repeated_int32 = []  # needs RepeatedField
    end

    assert_raise TypeError do
      m.repeated_int32.push "hello"
    end

    assert_raise TypeError do
      m.repeated_msg.push TestMessage.new
    end

    # TODO: UTF-8/ASCII checks
  end

  def test_rptfield_int32
    l = Google::Protobuf::RepeatedField.new(:int32)
    assert l.count == 0
    l = Google::Protobuf::RepeatedField.new(:int32, [1, 2, 3])
    assert l.count == 3
    assert l == [1, 2, 3]
    l.push 4
    assert l == [1, 2, 3, 4]
    dst_list = []
    l.each { |val| dst_list.push val }
    assert dst_list == [1, 2, 3, 4]
    assert l.to_a == [1, 2, 3, 4]
    assert l[0] == 1
    assert l[3] == 4
    l[0] = 5
    assert l == [5, 2, 3, 4]

    l2 = l.dup
    assert l == l2
    assert l.object_id != l2.object_id
    l2.push 6
    assert l.count == 4
    assert l2.count == 5

    assert l.inspect == '[5, 2, 3, 4]'

    l.insert(7, 8, 9)
    assert l == [5, 2, 3, 4, 7, 8, 9]
    assert l.pop == 9
    assert l == [5, 2, 3, 4, 7, 8]

    assert_raise TypeError do
      m = TestMessage.new
      l.push m
    end

    m = TestMessage.new
    m.repeated_int32 = l
    assert m.repeated_int32 == [5, 2, 3, 4, 7, 8]
    assert m.repeated_int32.object_id == l.object_id
    l.push 42
    assert m.repeated_int32.pop == 42
  end

  def test_rptfield_msg
    l = Google::Protobuf::RepeatedField.new(:message, TestMessage)
    l.push TestMessage.new
    assert l.count == 1
    assert_raise TypeError do
      l.push TestMessage2.new
    end
    assert_raise TypeError do
      l.push 42
    end

    l2 = l.dup
    assert l2[0] == l[0]
    assert l2[0].object_id != l[0].object_id
  end

  def test_rptfield_enum
    l = Google::Protobuf::RepeatedField.new(:enum, TestEnum)
    l.push :A
    l.push :B
    l.push :C
    assert l.count == 3
    assert_raise NameError do
      l.push :D
    end
    assert l[0] == :A

    l.push 4
    assert l[3] == 4
  end

  def test_rptfield_initialize
    assert_raise ArgumentError do
      l = Google::Protobuf::RepeatedField.new
    end
    assert_raise ArgumentError do
      l = Google::Protobuf::RepeatedField.new(:message)
    end
    assert_raise ArgumentError do
      l = Google::Protobuf::RepeatedField.new([1, 2, 3])
    end
    assert_raise ArgumentError do
      l = Google::Protobuf::RepeatedField.new(:message, [TestMessage2.new])
    end
  end

  def test_enum_field
    m = TestMessage.new
    assert m.optional_enum == :Default
    m.optional_enum = :A
    assert m.optional_enum == :A
    assert_raise NameError do
      m.optional_enum = :ASDF
    end
    m.optional_enum = 1
    assert m.optional_enum == :A
    m.optional_enum = 100
    assert m.optional_enum == 100
  end

  def test_dup
    m = TestMessage.new
    m.optional_string = "hello"
    m.optional_int32 = 42
    m.repeated_msg.push TestMessage2.new(:foo => 100)
    m.repeated_msg.push TestMessage2.new(:foo => 200)

    m2 = m.dup
    assert m == m2
    m.optional_int32 += 1
    assert m != m2
    assert m.repeated_msg[0] == m2.repeated_msg[0]
    assert m.repeated_msg[0].object_id != m2.repeated_msg[0].object_id
  end

  def test_enum_lookup
    assert TestEnum::A == 1
    assert TestEnum::B == 2
    assert TestEnum::C == 3

    assert TestEnum::lookup(1) == :A
    assert TestEnum::lookup(2) == :B
    assert TestEnum::lookup(3) == :C

    assert TestEnum::resolve(:A) == 1
    assert TestEnum::resolve(:B) == 2
    assert TestEnum::resolve(:C) == 3
  end

  def test_parse_serialize
    m = TestMessage.new(:optional_int32 => 42,
                        :optional_string => "hello world",
                        :optional_enum => :B,
                        :repeated_string => ["a", "b", "c"],
                        :repeated_int32 => [42, 43, 44],
                        :repeated_enum => [:A, :B, :C, 100],
                        :repeated_msg => [TestMessage2.new(:foo => 1), TestMessage2.new(:foo => 2)])
    data = TestMessage.encode m
    m2 = TestMessage.decode data
    assert m == m2
  end

  def test_def_errors
    s = Google::Protobuf::SymbolTable.new
    assert_raise TypeError do
      s.build do
        # enum with no default (integer value 0)
        add_enum "MyEnum" do
          value :A, 1
        end
      end
    end
    assert_raise TypeError do
      s.build do
        # message with required field (unsupported in proto3)
        add_message "MyMessage" do
          required :foo, :int32, 1
        end
      end
    end
  end

  def test_int_ranges
    m = TestMessage.new

    m.optional_int32 = 0
    m.optional_int32 = -0x8000_0000
    m.optional_int32 = +0x7fff_ffff
    m.optional_int32 = 1.0
    m.optional_int32 = -1.0
    m.optional_int32 = 2e9
    assert_raise RangeError do
      m.optional_int32 = -0x8000_0001
    end
    assert_raise RangeError do
      m.optional_int32 = +0x8000_0000
    end
    assert_raise RangeError do
      m.optional_int32 = +0x1000_0000_0000_0000_0000_0000 # force Bignum
    end
    assert_raise RangeError do
      m.optional_int32 = 1e12
    end
    assert_raise RangeError do
      m.optional_int32 = 1.5
    end

    m.optional_uint32 = 0
    m.optional_uint32 = +0xffff_ffff
    m.optional_uint32 = 1.0
    m.optional_uint32 = 4e9
    assert_raise RangeError do
      m.optional_uint32 = -1
    end
    assert_raise RangeError do
      m.optional_uint32 = -1.5
    end
    assert_raise RangeError do
      m.optional_uint32 = -1.5e12
    end
    assert_raise RangeError do
      m.optional_uint32 = -0x1000_0000_0000_0000
    end
    assert_raise RangeError do
      m.optional_uint32 = +0x1_0000_0000
    end
    assert_raise RangeError do
      m.optional_uint32 = +0x1000_0000_0000_0000_0000_0000 # force Bignum
    end
    assert_raise RangeError do
      m.optional_uint32 = 1e12
    end
    assert_raise RangeError do
      m.optional_uint32 = 1.5
    end

    m.optional_int64 = 0
    m.optional_int64 = -0x8000_0000_0000_0000
    m.optional_int64 = +0x7fff_ffff_ffff_ffff
    m.optional_int64 = 1.0
    m.optional_int64 = -1.0
    m.optional_int64 = 8e18
    m.optional_int64 = -8e18
    assert_raise RangeError do
      m.optional_int64 = -0x8000_0000_0000_0001
    end
    assert_raise RangeError do
      m.optional_int64 = +0x8000_0000_0000_0000
    end
    assert_raise RangeError do
      m.optional_int64 = +0x1000_0000_0000_0000_0000_0000 # force Bignum
    end
    assert_raise RangeError do
      m.optional_int64 = 1e50
    end
    assert_raise RangeError do
      m.optional_int64 = 1.5
    end

    m.optional_uint64 = 0
    m.optional_uint64 = +0xffff_ffff_ffff_ffff
    m.optional_uint64 = 1.0
    m.optional_uint64 = 16e18
    assert_raise RangeError do
      m.optional_uint64 = -1
    end
    assert_raise RangeError do
      m.optional_uint64 = -1.5
    end
    assert_raise RangeError do
      m.optional_uint64 = -1.5e12
    end
    assert_raise RangeError do
      m.optional_uint64 = -0x1_0000_0000_0000_0000
    end
    assert_raise RangeError do
      m.optional_uint64 = +0x1_0000_0000_0000_0000
    end
    assert_raise RangeError do
      m.optional_uint64 = +0x1000_0000_0000_0000_0000_0000 # force Bignum
    end
    assert_raise RangeError do
      m.optional_uint64 = 1e50
    end
    assert_raise RangeError do
      m.optional_uint64 = 1.5
    end

  end

  def test_stress_test
    m = TestMessage.new
    m.optional_int32 = 42
    m.optional_int64 = 0x100000000
    m.optional_string = "hello world"
    10.times do m.repeated_msg.push TestMessage2.new(:foo => 42) end
    10.times do m.repeated_string.push "hello world" end

    data = TestMessage.encode(m)

    l = 0
    10_000.times do
      m = TestMessage.decode(data)
      data_new = TestMessage.encode(m)
      assert data_new == data
      data = data_new
    end
  end

end
