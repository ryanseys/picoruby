module URI
  HEX = "0123456789ABCDEF"

  # Encode a single component as application/x-www-form-urlencoded.
  # Keeps alphanumerics and "*-._", maps space to "+", percent-encodes the rest
  # byte by byte. Matches CRuby's URI.encode_www_form_component output.
  def self.encode_www_form_component(str)
    result = ""
    str.to_s.each_byte do |b|
      if (0x30 <= b && b <= 0x39) || # 0-9
         (0x41 <= b && b <= 0x5A) || # A-Z
         (0x61 <= b && b <= 0x7A) || # a-z
         b == 0x2A || b == 0x2D || b == 0x2E || b == 0x5F # * - . _
        result << b.chr
      elsif b == 0x20 # space
        result << "+"
      else
        result << "%" << HEX[b >> 4] << HEX[b & 0x0F]
      end
    end
    result
  end

  # Encode an enumerable of key/value pairs (Hash or Array of pairs) as a
  # application/x-www-form-urlencoded query string. An Array value expands to
  # repeated keys; a nil value emits the bare key. Matches CRuby.
  def self.encode_www_form(enum)
    enum.map { |key, value|
      if value.nil?
        encode_www_form_component(key)
      elsif value.is_a?(Array)
        value.map { |v|
          pair = encode_www_form_component(key)
          pair << "=" << encode_www_form_component(v) unless v.nil?
          pair
        }.join("&")
      else
        "#{encode_www_form_component(key)}=#{encode_www_form_component(value)}"
      end
    }.join("&")
  end
end
