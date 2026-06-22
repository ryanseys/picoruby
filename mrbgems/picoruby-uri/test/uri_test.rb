class URITest < Picotest::Test
  def test_component_keeps_unreserved
    assert_equal "abcXYZ09", URI.encode_www_form_component("abcXYZ09")
    assert_equal "a*b-c.d_e", URI.encode_www_form_component("a*b-c.d_e")
  end

  def test_component_space_becomes_plus
    assert_equal "foo+bar", URI.encode_www_form_component("foo bar")
  end

  def test_component_percent_encodes_specials
    assert_equal "a%26b%3Dc", URI.encode_www_form_component("a&b=c")
    assert_equal "%2F%3F%23", URI.encode_www_form_component("/?#")
  end

  def test_component_accepts_non_string
    assert_equal "10", URI.encode_www_form_component(10)
  end

  def test_encode_www_form_hash
    assert_equal "q=ruby+on+rails&n=10",
      URI.encode_www_form({ "q" => "ruby on rails", "n" => 10 })
  end

  def test_encode_www_form_array_of_pairs
    assert_equal "a=1&b=2", URI.encode_www_form([["a", "1"], ["b", "2"]])
  end

  def test_encode_www_form_array_value_repeats_key
    assert_equal "a=1&a=2", URI.encode_www_form([["a", ["1", "2"]]])
  end

  def test_encode_www_form_nil_value_emits_bare_key
    assert_equal "a", URI.encode_www_form([["a", nil]])
  end
end
