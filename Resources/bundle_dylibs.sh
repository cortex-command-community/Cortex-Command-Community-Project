#!/bin/sh -eu

executable="${MESON_INSTALL_DESTDIR_PREFIX}/Contents/MacOS/${1}"
frameworks="${MESON_INSTALL_DESTDIR_PREFIX}/Contents/Frameworks"

dylibbundler -od -b -x "${executable}" \
 -d "${frameworks}" \
 -p "@executable_path/../Frameworks" \
 -s "${MESON_SOURCE_ROOT}/external/lib/macos" ${2}

# dylibbundler points every rpath at the Frameworks dir, which leaves duplicate LC_RPATH entries that dyld refuses to load on recent macOS.
for binary in "${executable}" "${frameworks}"/*.dylib; do
	duplicates=$(otool -l "${binary}" | awk '/cmd LC_RPATH/ { getline; getline; print $2 }' | sort | uniq -c | awk '$1 > 1')
	[ -n "${duplicates}" ] || continue
	echo "${duplicates}" | while read -r count rpath; do
		while [ "${count}" -gt 1 ]; do
			install_name_tool -delete_rpath "${rpath}" "${binary}"
			count=$((count - 1))
		done
	done
	# Editing the rpaths invalidates the ad-hoc signature dylibbundler applied, which arm64 requires to run.
	case " ${2} " in
		*" -ns "*) ;;
		*) codesign --force --sign - "${binary}" ;;
	esac
done
