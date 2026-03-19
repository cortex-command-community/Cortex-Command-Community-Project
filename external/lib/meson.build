if host_machine.system() == 'linux'
  subdir('linux')
elif host_machine.system() == 'windows'
  subdir('win')
elif host_machine.system() == 'darwin'
  subdir('macos')
elif host_machine.system() == 'emscripten'
  # No prebuilt native libs for Emscripten — all dependencies are ports or built from source
endif
