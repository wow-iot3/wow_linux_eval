│  LICENSE
│  README.md
│  
├─.vscode
│      settings.json
│      
├─3rdparty
│      readme.md
│      
├─build
│  │  envsetup.sh
│  │  profile.sh
│  │  readme.md
│  │  
│  ├─core
│  │      build_executable.mk
│  │      build_fromsrc.mk
│  │      build_fromtar.mk
│  │      build_library.mk
│  │      build_qtproject.mk
│  │      build_resource.mk
│  │      clear_vars.mk
│  │      dep_file.mk
│  │      main.mk
│  │      mbuild-core.sh
│  │      
│  └─templates
│          autobuild.sh.in
│          CMakeLists.txt.in
│          from-src.mk.in
│          from-tar.mk.in
│          Makefile.mbuild.in
│          target-exec.mk.in
│          target-lib.mk.in
│          target-qt.mk.in
│          target-res.mk.in
│          
├─doc
│  │  change-log.md
│  │  code-rule.md
│  │  source-tree.md
│  │  update-log.md
│  │  
│  └─wow_base
│          wow-bype文件说明.md
│          wow-check文件说明.md
│          wow-cpu文件说明.md
│          wow-type文件说明.md
│          
├─wow_app
│      readme.md
│      
├─wow_base
│  │  readme.md
│  │  wow_base_src.mk
│  │  wow_base_test.mk
│  │  
│  ├─inc
│  │  ├─prefix
│  │  │      wow_byte.h
│  │  │      wow_check.h
│  │  │      wow_common.h
│  │  │      wow_cpu.h
│  │  │      wow_errno.h
│  │  │      wow_keyword.h
│  │  │      
│  │  └─utils
│  │          wow_type.h
│  │          
│  ├─src
│  │  └─utils
│  │          wow_type.c
│  │          
│  └─test
│      │  main.c
│      │  
│      ├─greatest
│      │      greatest.h
│      │      
│      └─utils
│              suit_type.c
│              
├─wow_daemon
│      readme.md
│      
├─wow_hal
│      readme.md
│      
├─wow_iot
│      readme.md
│      
├─wow_show
│      readme.md
│      
└─wow_tools
    │  readme.md
    │  
    └─appendcrc
            appendcrc.mk
            wow_append_crc.c
