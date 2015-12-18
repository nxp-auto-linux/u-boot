use Digest::CMAC;

#AVK Key
my $key = "\xc5\xe8\xd1\x74\x1b\xa3\x39\xb9\x85\xeb\x03\x67\xf3\x2f\xf7\x7c";
my $plaintext = <STDIN>;

my $alg = Digest::CMAC->new($key);
$alg->add($plaintext);
my $result = $alg->digest;
print $result
