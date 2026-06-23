class Data
  # The methods below are inherited by every value class built with
  # Data.define. Data.define itself returns a Data instance (the value class
  # holder) that has no members, so each method falls back to a plain default
  # unless `self` is an actual value instance (responds to to_h).

  # Value equality: same class and equal members.
  def ==(other)
    return object_id == other.object_id unless respond_to?(:to_h)
    self.class == other.class && other.respond_to?(:to_h) && to_h == other.to_h
  end
  alias eql? ==

  # Hash derived from the inspect string so it stays consistent with ==
  # without depending on per-type #hash (absent for Integer on some VMs).
  def hash
    return object_id unless respond_to?(:to_h)
    h = 0
    inspect.each_byte { |b| h = (h * 31 + b) & 0x3fffffff }
    h
  end

  def deconstruct
    to_h.values
  end

  def deconstruct_keys(keys)
    return to_h if keys.nil?
    names = members
    result = {}
    keys.each { |k| result[k] = to_h[k] if names.include?(k) }
    result
  end

  def inspect
    return "#<data>" unless respond_to?(:to_h)
    pairs = to_h.map { |k, v| "#{k}=#{v.inspect}" }.join(", ")
    "#<data #{pairs}>"
  end
  alias to_s inspect
end
