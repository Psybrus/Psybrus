%define pref /usr
%define ver  2.8.12
%define ver2 2.8
%define rel  0

# Configurable settings (use --with(out) unicode on rpmbuild cmd line):
%define unicode 0
%{?_with_unicode: %{expand: %%define unicode 1}}
%{?_without_unicode: %{expand: %%define unicode 0}}

%define portname x11univ

%if %{unicode}
    %define wxbasename   wx-base-unicode
    %define name         wx-%{portname}-unicode
    %define wxconfig     %{portname}-unicode-release-%{ver2}
    %define wxconfiglink wx%{portname}u-%{ver2}-config
%else
    %define wxbasename   wx-base
    %define name         wx-%{portname}
    %define wxconfig     %{portname}-ansi-release-%{ver2}
    %define wxconfiglink wx%{portname}-%{ver2}-config
%endif

Summary: The X11 port of the wxWidgets library
Name: %{name}
Version: %{ver}
Release: %{rel}
License: wxWindows Licence
Group: X11/Libraries
Source: wxX11-%{ver}.tar.bz2
URL: http://www.wxwidgets.org
Packager: Vadim Zeitlin <vadim@wxwindows.org>
Prefix: %{pref}
BuildRoot: %{_tmppath}/%{name}-root
Requires: %{wxbasename} = %{ver}

# all packages providing an implementation of wxWidgets library (regardless of
# the toolkit used) should provide the (virtual) wxwin package, this makes it
# possible to require wxwin instead of requiring "wxgtk or wxmotif or wxuniv..."
Provides: wxwin
Provides: wxX11

%description
wxWidgets is a free C++ library for cross-platform GUI development.
With wxWidgets, you can create applications for different GUIs (GTK+,
Motif/LessTif, MS Windows, Mac) from the same source code.

%package devel
Summary: The X11 port of the wxWidgets library
Group: X11/Libraries
Requires: %{name} = %{ver}
Requires: %{wxbasename}-devel = %{ver}

%description devel
Header files for wxX11, the X11 port of the wxWidgets library.

%package gl
Summary: The X11 port of the wxWidgets library, OpenGL add-on.
Group: X11/Libraries
Requires: %{name} = %{ver}

%description gl
OpenGL add-on library for wxX11, the X11 port of the wxWidgets library.

%package static
Summary: wxX11 static libraries
Group: Development/Libraries
Requires: %{wxbasename}-static
Requires: %{name}-devel = %{ver}

%description static
Static libraries for wxX11. You need them if you want to link statically against wxX11.

%package contrib
Summary: Contrib libraries for wxX11.
Group: X11/Libraries
Requires: %{name} = %{ver}

%description contrib
Contributed libraries for wxX11.

%package contrib-devel
Summary: Contrib libraries for wxX11.
Group: X11/Libraries
Requires: %{name}-contrib = %{ver}
Requires: %{name}-devel = %{ver}

%description contrib-devel
Header files for contributed libraries for wxX11.

%prep
%setup -q -n wxX11-%{ver}

%build
if [ "$SMP" != "" ]; then
    export MAKE="make -j$SMP"
else
    export MAKE="make"
fi

mkdir obj-shared
cd obj-shared
../configure --prefix=%{pref} --with-x11 \
%if %{unicode}
                              --enable-unicode \
%else
                              --with-odbc \
%endif
                              --with-opengl
$MAKE

cd contrib/src
$MAKE
cd ../../..

mkdir obj-static
cd obj-static
../configure --prefix=%{pref} --with-x11 --disable-shared \
%if %{unicode}
                              --enable-unicode \
%else
                              --with-odbc \
%endif
                              --with-opengl
$MAKE
cd ..

%install
rm -rf $RPM_BUILD_ROOT
(cd obj-static; make prefix=$RPM_BUILD_ROOT%{pref} install)
(cd obj-shared; make prefix=$RPM_BUILD_ROOT%{pref} install)

# Remove headers that are part of wx-base-devel:

# --- wxBase headers list begins here ---
cat <<EOF >wxbase-headers.files
wx/afterstd.h
wx/app.h
wx/apptrait.h
wx/archive.h
wx/arrimpl.cpp
wx/arrstr.h
wx/beforestd.h
wx/buffer.h
wx/build.h
wx/chkconf.h
wx/clntdata.h
wx/cmdline.h
wx/confbase.h
wx/config.h
wx/convauto.h
wx/containr.h
wx/cpp.h
wx/datetime.h
wx/datstrm.h
wx/dde.h
wx/debug.h
wx/defs.h
wx/dir.h
wx/dlimpexp.h
wx/dynarray.h
wx/dynlib.h
wx/dynload.h
wx/encconv.h
wx/event.h
wx/except.h
wx/features.h
wx/ffile.h
wx/file.h
wx/fileconf.h
wx/filefn.h
wx/filename.h
wx/filesys.h
wx/fontenc.h
wx/fontmap.h
wx/fs_arc.h
wx/fs_filter.h
wx/fs_mem.h
wx/fs_zip.h
wx/hash.h
wx/hashmap.h
wx/hashset.h
wx/html/forcelnk.h
wx/iconloc.h
wx/init.h
wx/intl.h
wx/iosfwrap.h
wx/ioswrap.h
wx/ipc.h
wx/ipcbase.h
wx/isql.h
wx/isqlext.h
wx/link.h
wx/list.h
wx/listimpl.cpp
wx/log.h
wx/longlong.h
wx/math.h
wx/memconf.h
wx/memory.h
wx/memtext.h
wx/mimetype.h
wx/module.h
wx/msgout.h
wx/mstream.h
wx/object.h
wx/platform.h
wx/platinfo.h
wx/power.h
wx/process.h
wx/ptr_scpd.h
wx/recguard.h
wx/regex.h
wx/scopeguard.h
wx/snglinst.h
wx/sstream.h
wx/stack.h
wx/stackwalk.h
wx/stdpaths.h
wx/stockitem.h
wx/stopwatch.h
wx/strconv.h
wx/stream.h
wx/string.h
wx/sysopt.h
wx/tarstrm.h
wx/textbuf.h
wx/textfile.h
wx/thread.h
wx/thrimpl.cpp
wx/timer.h
wx/tokenzr.h
wx/txtstrm.h
wx/types.h
wx/uri.h
wx/utils.h
wx/variant.h
wx/vector.h
wx/version.h
wx/volume.h
wx/wfstream.h
wx/wx.h
wx/wxchar.h
wx/wxprec.h
wx/xti.h
wx/xtistrm.h
wx/zipstrm.h
wx/zstream.h
wx/unix/apptbase.h
wx/unix/apptrait.h
wx/unix/execute.h
wx/unix/mimetype.h
wx/unix/pipe.h
wx/unix/private.h
wx/unix/stackwalk.h
wx/unix/stdpaths.h
wx/fs_inet.h
wx/gsocket.h
wx/protocol/file.h
wx/protocol/ftp.h
wx/protocol/http.h
wx/protocol/protocol.h
wx/sckaddr.h
wx/sckipc.h
wx/sckstrm.h
wx/socket.h
wx/url.h
wx/unix/gsockunx.h
wx/xml/xml.h
wx/xtixml.h
wx/db.h
wx/dbkeyg.h
wx/dbtable.h
EOF
# --- wxBase headers list ends here ---
for f in `cat wxbase-headers-list` ; do
  rm -f $RPM_BUILD_ROOT%{_includedir}/wx-%{ver2}/$f
done

# list of all core headers:
find $RPM_BUILD_ROOT%{_includedir}/wx-%{ver2} -type f | sed -e "s,$RPM_BUILD_ROOT,,g" >core-headers.files

# contrib stuff:
(cd obj-shared/contrib/src; make prefix=$RPM_BUILD_ROOT%{pref} install)
(cd obj-shared/utils/wxrc; make prefix=$RPM_BUILD_ROOT%{pref} install)

# remove wxBase files so that RPM doesn't complain about unpackaged files:
rm -f $RPM_BUILD_ROOT%{_libdir}/libwx_base*
%if %{unicode}
    rm -f $RPM_BUILD_ROOT%{_libdir}/libwxregexu-%{ver2}.a
%endif
rm -f $RPM_BUILD_ROOT%{_datadir}/aclocal/*
rm -f $RPM_BUILD_ROOT%{_datadir}/locale/*/*/*

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%post devel
# link wx-config when you install RPM.
ln -sf %{_libdir}/wx/config/%{wxconfig} %{_bindir}/wx-config
# link wx-config with explicit name.
ln -sf %{_libdir}/wx/config/%{wxconfig} %{_bindir}/%{wxconfiglink}
/sbin/ldconfig

%postun devel
/sbin/ldconfig

%preun devel
if test -f %{_bindir}/wx-config -a -f /usr/bin/md5sum ; then
    SUM1=`md5sum %{_libdir}/wx/config/%{wxconfig} | cut -c 0-32`
    SUM2=`md5sum %{_bindir}/wx-config | cut -c 0-32`
    if test "x$SUM1" = "x$SUM2" ; then
        rm -f %{_bindir}/wx-config
    fi
fi
rm -f %{_bindir}/%{wxconfiglink}

%post gl
/sbin/ldconfig

%postun gl
/sbin/ldconfig

%post contrib
/sbin/ldconfig

%postun contrib
/sbin/ldconfig

%post contrib-devel
/sbin/ldconfig

%postun contrib-devel
/sbin/ldconfig

%files
%defattr(-,root,root)
%doc COPYING.LIB *.txt
%{_libdir}/libwx_%{portname}*_adv-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_core-%{ver2}.so.*
%if !%{unicode}
    %{_libdir}/libwx_%{portname}*_dbgrid-%{ver2}.so.*
%endif
%{_libdir}/libwx_%{portname}*_html-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_media-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_qa-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_xrc-%{ver2}.so.*


%files devel -f core-headers.files
%defattr(-,root,root)
%{_libdir}/libwx_%{portname}*_adv-%{ver2}.so
%{_libdir}/libwx_%{portname}*_core-%{ver2}.so
%if !%{unicode}
    %{_libdir}/libwx_%{portname}*_dbgrid-%{ver2}.so
%endif
%{_libdir}/libwx_%{portname}*_gl-%{ver2}.so
%{_libdir}/libwx_%{portname}*_html-%{ver2}.so
%{_libdir}/libwx_%{portname}*_media-%{ver2}.so
%{_libdir}/libwx_%{portname}*_qa-%{ver2}.so
%{_libdir}/libwx_%{portname}*_xrc-%{ver2}.so
%dir %{_libdir}/wx
%{_libdir}/wx/*
%{_bindir}/wxrc*

%files gl
%defattr(-,root,root)
%{_libdir}/libwx_%{portname}*_gl-%{ver2}.so.*

%files static
%defattr (-,root,root)
%{_libdir}/libwx_%{portname}*_*-%{ver2}.a

%files contrib
%defattr(-,root,root)
%{_libdir}/libwx_%{portname}*_animate-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_deprecated-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_fl-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_gizmos-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_mmedia-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_ogl-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_plot-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_stc-%{ver2}.so.*
%{_libdir}/libwx_%{portname}*_svg-%{ver2}.so.*

%files contrib-devel
%defattr(-,root,root)
%dir %{_includedir}/wx-%{ver2}/wx/animate
%{_includedir}/wx-%{ver2}/wx/animate/*
%{_libdir}/libwx_%{portname}*_animate-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/deprecated
%{_includedir}/wx-%{ver2}/wx/deprecated/*
%{_libdir}/libwx_%{portname}*_deprecated-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/fl
%{_includedir}/wx-%{ver2}/wx/fl/*
%{_libdir}/libwx_%{portname}*_fl-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/gizmos
%{_includedir}/wx-%{ver2}/wx/gizmos/*
%{_libdir}/libwx_%{portname}*_gizmos-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/mmedia
%{_includedir}/wx-%{ver2}/wx/mmedia/*
%{_libdir}/libwx_%{portname}*_mmedia-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/ogl
%{_includedir}/wx-%{ver2}/wx/ogl/*
%{_libdir}/libwx_%{portname}*_ogl-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/plot
%{_includedir}/wx-%{ver2}/wx/plot/*
%{_libdir}/libwx_%{portname}*_plot-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/stc
%{_includedir}/wx-%{ver2}/wx/stc/*
%{_libdir}/libwx_%{portname}*_stc-%{ver2}.so

%dir %{_includedir}/wx-%{ver2}/wx/svg
%{_includedir}/wx-%{ver2}/wx/svg/*
%{_libdir}/libwx_%{portname}*_svg-%{ver2}.so
