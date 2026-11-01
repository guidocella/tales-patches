EXPORTS = GetFileVersionInfoA GetFileVersionInfoExA GetFileVersionInfoExW \
	GetFileVersionInfoSizeA GetFileVersionInfoSizeExA GetFileVersionInfoSizeExW \
	GetFileVersionInfoSizeW GetFileVersionInfoW VerFindFileA VerFindFileW \
	VerInstallFileA VerInstallFileW VerLanguageNameA VerLanguageNameW \
	VerQueryValueA VerQueryValueW

version.dll: sempool.o version.res kernel32.lib
	lld-link /dll /nodefaultlib /entry:DllMain /out:$@ sempool.o version.res kernel32.lib $(EXPORTS:%=/export:%)

version.res: version.rc
	llvm-rc /fo $@ $<

sempool.o: sempool.c
	clang --target=x86_64-pc-windows-msvc -O2 -ffreestanding -fno-builtin -fno-stack-protector -mno-stack-arg-probe -Wall -c -o $@ $<

kernel32.lib: kernel32.def
	llvm-dlltool -m i386:x86-64 -d $< -l $@

clean:
	rm -f version.dll version.lib version.exp version.res sempool.o kernel32.lib

.PHONY: clean
