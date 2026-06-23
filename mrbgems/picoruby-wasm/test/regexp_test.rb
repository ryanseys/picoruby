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
end
