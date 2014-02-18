# coding: utf-8
Gem::Specification.new do |spec|
  spec.name          = "extreme_timeout"
  spec.version       = "0.3.1"
  spec.authors       = ["Masaya SUZUKI"]
  spec.email         = ["draftcode@gmail.com"]
  spec.description   = "Timeout from the outside of the GVL"
  spec.summary       = "Timeout from the outside of the GVL"
  spec.homepage      = "https://github.com/draftcode/extreme_timeout"
  spec.license       = "MIT"

  spec.files         = `git ls-files`.split($/)
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.extensions    = ["ext/extreme_timeout/extconf.rb"]

  spec.add_development_dependency "bundler", "~> 1.3"
  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler"
  spec.add_development_dependency "rspec"
end
