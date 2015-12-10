require 'cmac-rb'

#AVK Key
key="\xc5\xe8\xd1\x74\x1b\xa3\x39\xb9\x85\xeb\x03\x67\xf3\x2f\xf7\x7c"
plaintext = gets

digest = CMAC::Digest.new(key)
result = digest.update(plaintext)
print result
