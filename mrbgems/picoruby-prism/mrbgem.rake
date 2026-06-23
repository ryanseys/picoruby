MRuby::Gem::Specification.new('picoruby-prism') do |spec|
  spec.license = 'MIT'
  spec.author  = 'HASUMI Hitoshi'
  spec.summary = 'Prism universal parser binding'

  cc.include_paths << "#{dir}/include"

  if build.vm_mruby?
    # Reuse the prism library already compiled and linked by mruby-compiler
    # (standard allocator) instead of building a second copy of libprism.
    spec.add_dependency 'mruby-compiler'
    cc.include_paths << "#{dir}/../mruby-compiler/lib/prism/include"
  else
    lib_dir = "#{dir}/lib"
    prism_dir = "#{lib_dir}/prism"
    cc.include_paths << "#{prism_dir}/include"
    cc.defines << 'PRISM_CUSTOM_ALLOCATOR'

    directory prism_dir do
      FileUtils.cd lib_dir do
        sh "git clone https://github.com/hasumikin/prism.git"
      end
      FileUtils.cd prism_dir do
        sh "git checkout custom_allocation_interface"
      end
    end

    task :prism_templates => prism_dir do
      FileUtils.cd prism_dir do
        sh "templates/template.rb"
      end
    end

    Rake::Task[:prism_templates].invoke

    Dir.glob("#{prism_dir}/src/**/*.c").map do |src|
      obj = objfile(src.pathmap("#{build_dir}/lib/%n"))
      build.libmruby_objs << obj
      task obj => [src] do |f|
        cc.run f.name, f.prerequisites.first
      end
    end

    task :deep_clean do
      rm_rf prism_dir
    end
  end
end
