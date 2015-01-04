#!/usr/bin/ruby

require 'google/protobuf'
require 'test/unit'

# ------------- generated code --------------

module BasicTest
  descriptor = Google::Protobuf::DescriptorPool.new('basic.Basic')

  TestMessage = descriptor.lookup("TestMessage")
  #TestMessage2 = pool.lookup("TestMessage2").msgclass
  #Recursive1 = pool.lookup("Recursive1").msgclass
  #Recursive2 = pool.lookup("Recursive2").msgclass
  #TestEnum = pool.lookup("TestEnum").enummodule
  #BadFieldNames = pool.lookup("BadFieldNames").msgclass

# ------------ test cases ---------------

  class MessageContainerTest < Test::Unit::TestCase

    def test_defaults
      m = TestMessage.new
      assert_equal 0, m.optional_int32
      assert_equal 0, m.optional_int64
      assert_equal 0, m.optional_uint32
      assert_equal 0, m.optional_uint64
      assert_equal false, m.optional_bool
      assert_equal 0.0, m.optional_float
      assert_equal 0.0, m.optional_double
      assert_equal "", m.optional_string
      assert_equal "", m.optional_bytes
      assert_equal nil, m.optional_msg
      assert_equal :Default, m.optional_enum
    end

    def test_setters
      m = TestMessage.new
      m.optional_int32 = -42
      assert_equal -42, m.optional_int32
      m.optional_int64 = -0x1_0000_0000
      assert_equal -0x1_0000_0000, m.optional_int64
      m.optional_uint32 = 0x9000_0000
      assert_equal 0x9000_0000, m.optional_uint32
      m.optional_uint64 = 0x9000_0000_0000_0000
      assert_equal 0x9000_0000_0000_0000, m.optional_uint64
      m.optional_bool = true
      assert_equal true, m.optional_bool
      m.optional_float = 0.5
      assert_equal 0.5, m.optional_float
      m.optional_double = 0.5
      m.optional_string = "hello"
      assert_equal "hello", m.optional_string
      m.optional_bytes = "world".encode!('ASCII-8BIT')
      assert_equal "world", m.optional_bytes
      m.optional_msg = TestMessage2.new(:foo => 42)
      assert_equal TestMessage2.new(:foo => 42), m.optional_msg
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
      expected = '<BasicTest::TestMessage: optional_int32: -42, optional_int64: 0, optional_uint32: 0, optional_uint64: 0, optional_bool: false, optional_float: 0.0, optional_double: 0.0, optional_string: "", optional_bytes: "", optional_msg: <BasicTest::TestMessage2: foo: 0>, optional_enum: :A, repeated_int32: [], repeated_int64: [], repeated_uint32: [], repeated_uint64: [], repeated_bool: [], repeated_float: [], repeated_double: [], repeated_string: ["hello", "there", "world"], repeated_bytes: [], repeated_msg: [], repeated_enum: []>'
      assert m.inspect == expected
    end

    def test_hash
      m1 = TestMessage.new(:optional_int32 => 42)
      m2 = TestMessage.new(:optional_int32 => 102)
      assert m1.hash != 0
      assert m2.hash != 0
      # relying on the randomness here -- if hash function changes and we are
      # unlucky enough to get a collision, then change the values above.
      assert m1.hash != m2.hash
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
    end

    def test_string_encoding
      m = TestMessage.new

      # Assigning a normal (ASCII or UTF8) string to a bytes field, or
      # ASCII-8BIT to a string field, raises an error.
      assert_raise TypeError do
        m.optional_bytes = "Test string ASCII".encode!('ASCII')
      end
      assert_raise TypeError do
        m.optional_bytes = "Test string UTF-8 \u0100".encode!('UTF-8')
      end
      assert_raise TypeError do
        m.optional_string = ["FFFF"].pack('H*')
      end

      # "Ordinary" use case.
      m.optional_bytes = ["FFFF"].pack('H*')
      m.optional_string = "\u0100"

      # strings are mutable so we can do this, but serialize should catch it.
      m.optional_string = "asdf".encode!('UTF-8')
      m.optional_string.encode!('ASCII-8BIT')
      assert_raise TypeError do
        data = TestMessage.encode(m)
      end
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

      l3 = l + l.dup
      assert l3.count == l.count * 2
      l.count.times do |i|
        assert l3[i] == l[i]
        assert l3[l.count + i] == l[i]
      end

      l.clear
      assert l.count == 0
      l += [1, 2, 3, 4]
      l.replace([5, 6, 7, 8])
      assert l == [5, 6, 7, 8]

      l4 = Google::Protobuf::RepeatedField.new(:int32)
      l4[5] = 42
      assert l4 == [0, 0, 0, 0, 0, 42]

      l4 << 100
      assert l4 == [0, 0, 0, 0, 0, 42, 100]
      l4 << 101 << 102
      assert l4 == [0, 0, 0, 0, 0, 42, 100, 101, 102]
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
      assert l2[0].object_id == l[0].object_id

      l2 = Google::Protobuf.deep_copy(l)
      assert l2[0] == l[0]
      assert l2[0].object_id != l[0].object_id

      l3 = l + l2
      assert l3.count == 2
      assert l3[0] == l[0]
      assert l3[1] == l2[0]
      l3[0].optional_int32 = 1000
      assert l[0].optional_int32 == 1000

      new_msg = TestMessage.new(:optional_int32 => 200)
      l4 = l + [new_msg]
      assert l4.count == 2
      new_msg.optional_int32 = 1000
      assert l4[1].optional_int32 == 1000
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
      assert m.repeated_msg[0].object_id == m2.repeated_msg[0].object_id
    end

    def test_deep_copy
      m = TestMessage.new(:optional_int32 => 42,
                          :repeated_msg => [TestMessage2.new(:foo => 100)])
      m2 = Google::Protobuf.deep_copy(m)
      assert m == m2
      assert m.repeated_msg == m2.repeated_msg
      assert m.repeated_msg.object_id != m2.repeated_msg.object_id
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

      data = Google::Protobuf.encode m
      m2 = Google::Protobuf.decode(TestMessage, data)
      assert m == m2
    end

    def test_def_errors
      s = Google::Protobuf::DescriptorPool.new
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

    def test_corecursive
      # just be sure that we can instantiate types with corecursive field-type
      # references.
      m = Recursive1.new(:foo => Recursive2.new(:foo => Recursive1.new))
      assert Recursive1.descriptor.lookup("foo").subtype ==
        Recursive2.descriptor
      assert Recursive2.descriptor.lookup("foo").subtype ==
        Recursive1.descriptor

      serialized = Recursive1.encode(m)
      m2 = Recursive1.decode(serialized)
      assert m == m2
    end

    def test_serialize_cycle
      m = Recursive1.new(:foo => Recursive2.new)
      m.foo.foo = m
      assert_raise RuntimeError do
        serialized = Recursive1.encode(m)
      end
    end

    def test_bad_field_names
      m = BadFieldNames.new(:dup => 1, :class => 2)
      m2 = m.dup
      assert m == m2
      assert m['dup'] == 1
      assert m['class'] == 2
      m['dup'] = 3
      assert m['dup'] == 3
      m['a.b'] = 4
      assert m['a.b'] == 4
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

    def test_reflection
      m = TestMessage.new(:optional_int32 => 1234)
      msgdef = m.class.descriptor
      assert msgdef.class == Google::Protobuf::Descriptor
      assert msgdef.any? {|field| field.name == "optional_int32"}
      optional_int32 = msgdef.lookup "optional_int32"
      assert optional_int32.class == Google::Protobuf::FieldDescriptor
      assert optional_int32 != nil
      assert optional_int32.name == "optional_int32"
      assert optional_int32.type == :int32
      optional_int32.set(m, 5678)
      assert m.optional_int32 == 5678
      m.optional_int32 = 1000
      assert optional_int32.get(m) == 1000

      optional_msg = msgdef.lookup "optional_msg"
      assert optional_msg.subtype == TestMessage2.descriptor

      optional_msg.set(m, optional_msg.subtype.msgclass.new)

      assert msgdef.msgclass == TestMessage

      optional_enum = msgdef.lookup "optional_enum"
      assert optional_enum.subtype == TestEnum.descriptor
      assert optional_enum.subtype.class == Google::Protobuf::EnumDescriptor
      optional_enum.subtype.each do |k, v|
        # set with integer, check resolution to symbolic name
        optional_enum.set(m, v)
        assert optional_enum.get(m) == k
      end
    end

    def test_json
      m = TestMessage.new(:optional_int32 => 1234,
                          :optional_int64 => -0x1_0000_0000,
                          :optional_uint32 => 0x8000_0000,
                          :optional_uint64 => 0xffff_ffff_ffff_ffff,
                          :optional_bool => true,
                          :optional_float => 1.0,
                          :optional_double => -1e100,
                          :optional_string => "Test string",
                          :optional_bytes => ["FFFFFFFF"].pack('H*'),
                          :optional_msg => TestMessage2.new(:foo => 42),
                          :repeated_int32 => [1, 2, 3, 4],
                          :repeated_string => ["a", "b", "c"],
                          :repeated_bool => [true, false, true, false],
                          :repeated_msg => [TestMessage2.new(:foo => 1),
                                            TestMessage2.new(:foo => 2)])

      json_text = TestMessage.encode_json(m)
      m2 = TestMessage.decode_json(json_text)
      assert m == m2
    end
  end
end
