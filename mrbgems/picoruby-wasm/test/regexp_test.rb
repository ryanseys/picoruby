class RegexpExtTest < Picotest::Test
  # ---- String#gsub / #sub with a String pattern match literally ----

  def test_gsub_string_pattern_is_literal
    assert_equal "a-b-c", "a.b.c".gsub(".", "-")
    assert_equal "aXbXc", "a.b.c".gsub(".", "X")
  end

  def test_gsub_string_pattern_with_metachars
    assert_equal "a b", "a(x)b".gsub("(x)", " ")
  end

  def test_sub_string_pattern_is_literal
    assert_equal "a-b.c", "a.b.c".sub(".", "-")
  end

  # ---- String#gsub with a Hash replacement ----

  def test_gsub_hash_replacement
    assert_equal "&lt;x&gt;", "<x>".gsub(/[<>]/, { "<" => "&lt;", ">" => "&gt;" })
  end

  def test_gsub_hash_missing_key_deletes
    assert_equal "ac", "abc".gsub(/[abc]/, { "a" => "a", "c" => "c" })
  end

  # ---- Ruby-only regex escapes accepted ----

  def test_anchors_A_z
    assert_true "hello".match?(/\A\w+\z/)
    assert_false "hello\nx".match?(/\A\w+\z/)
  end

  def test_escape_e_is_esc
    assert_true "\e[0m".match?(/\e\[[\d;]*m/)
  end

  def test_hex_shorthand
    assert_true "1aF".match?(/\A\h+\z/)
    assert_false "1ag".match?(/\A\h+\z/)
  end

  # ---- capture globals ----

  def test_match_globals_after_match_op
    "hi" =~ /(\w)(\w)/
    assert_not_nil $~
    assert_equal "h", $1
    assert_equal "i", $2
    assert_nil $3
  end

  def test_match_globals_cleared_on_no_match
    "hi" =~ /(\w)/
    "zz" =~ /(\d)/
    assert_nil $~
    assert_nil $1
  end

  def test_gsub_block_sees_dollar_one
    out = "ab".gsub(/(.)/) { $1.upcase }
    assert_equal "AB", out
  end

  def test_last_match
    "hi" =~ /(\w)(\w)/
    assert_equal "hi", Regexp.last_match[0]
    assert_equal "h", Regexp.last_match(1)
    assert_equal "i", Regexp.last_match(2)
  end

  # ---- String#scan ----

  def test_scan_no_groups
    assert_equal ["a", "b", "c"], "a1b2c".scan(/[a-z]/)
  end

  def test_scan_with_groups
    assert_equal [["a", "1"], ["b", "2"]], "a1b2".scan(/([a-z])(\d)/)
  end

  def test_scan_block
    seen = []
    "a1b2".scan(/[a-z]/) { |m| seen << m }
    assert_equal ["a", "b"], seen
  end

  def test_scan_literal_string
    assert_equal [".", "."], "a.b.c".scan(".")
  end
end
