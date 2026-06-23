class PrismTest < Picotest::Test
  # ---- Prism.parse ----
  # Returns the pretty-printed AST dump. When the gem reuses the size-optimized
  # prism from mruby-compiler (PRISM_EXCLUDE_PRETTYPRINT), the dump is empty;
  # use Prism.lex / Prism.errors instead. Either way it returns a String.

  def test_parse_returns_string
    assert_true Prism.parse("1 + 2").is_a?(String)
  end

  # ---- Prism.errors ----

  def test_errors_empty_for_valid_source
    assert_equal [], Prism.errors("a = 1")
  end

  def test_errors_reports_syntax_error
    errs = Prism.errors("def")
    assert_true errs.size > 0
    e = errs[0]
    assert_true e[:message].is_a?(String)
    assert_true e[:line].is_a?(Integer)
    assert_true e[:column].is_a?(Integer)
  end

  # ---- Prism.lex ----

  def test_lex_returns_token_triples
    toks = Prism.lex("foo")
    assert_true toks.is_a?(Array)
    assert_true toks.size > 0
    first = toks[0]
    assert_true first[0].is_a?(Symbol)
    assert_equal 0, first[1]
    assert_equal 3, first[2]
  end

  def test_lex_offsets_advance
    toks = Prism.lex("a b")
    # at least two identifier tokens at distinct offsets
    starts = toks.map { |t| t[1] }
    assert_true starts.include?(0)
    assert_true starts.include?(2)
  end
end
