%define __spec_install_post %{nil}
%define debug_package %{nil}
%define __os_install_post %{_dbpath}/brp-compress
%define not_rpmdir ./
%define _build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm
%{!?build_version: %define build_version 0.0.0}

Summary: Collection of video tools
Name: mistserver
Version: %{build_version}
Release: %{release}.el9
License: UNLICENSE
Group: Applications/Multimedia
URL: https://github.com/DDVTECH/mistserver

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: systemd systemd-rpm-macros
#BuildArch: noarch

Requires: systemd

%description
MistServer streaming server.

%prep
# nothing to prep

%build
meson setup -DWITH_AV=true -DWITH_JPG=true -DNOGA=true -Dprefix=/usr %{_builddir} %{_sourcedir}
meson compile -C %{_builddir}

%install
rm -rf %{buildroot}
DESTDIR=%{buildroot} meson install -C %{_builddir}
strip %{buildroot}%{_bindir}/*
strip %{buildroot}%{_libdir}/*

install --mode=644 -D %{_sourcedir}/mistserver.service      %{buildroot}%{_unitdir}/mistserver.service
install --mode=644 -D %{_sourcedir}/UNLICENSE               %{buildroot}/usr/share/licenses/mistserver/UNLICENSE
install --mode=644 -D %{_sourcedir}/COPYING.md              %{buildroot}/usr/share/licenses/mistserver/COPYING.md

echo "{}" >%{_builddir}/mistserver.conf 
install --mode=644 -D %{_builddir}/mistserver.conf      %{buildroot}%{_sysconfdir}/mistserver.conf

%clean
# Make a tarball copy before cleanup
rm -rf %{buildroot}

%post
%systemd_post mistserver.service

%preun
%systemd_preun mistserver.service

%postun
%systemd_postun_with_restart mistserver.service

%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/mistserver.conf
%{_bindir}/MistAnalyserDTSC
%{_bindir}/MistAnalyserEBML
%{_bindir}/MistAnalyserFLV
%{_bindir}/MistAnalyserH264
%{_bindir}/MistAnalyserHLS
%{_bindir}/MistAnalyserMP4
%{_bindir}/MistAnalyserOGG
%{_bindir}/MistAnalyserRIFF
%{_bindir}/MistAnalyserRTMP
%{_bindir}/MistAnalyserRTSP
%{_bindir}/MistAnalyserTS
%{_bindir}/MistController
%{_bindir}/MistInAAC
%{_bindir}/MistInBalancer
%{_bindir}/MistInBuffer
%{_bindir}/MistInDTSC
%{_bindir}/MistInEBML
%{_bindir}/MistInFLV
%{_bindir}/MistInFolder
%{_bindir}/MistInH264
%{_bindir}/MistInHLS
%{_bindir}/MistInISMV
%{_bindir}/MistInMP3
%{_bindir}/MistInMP4
%{_bindir}/MistInOGG
%{_bindir}/MistInPlaylist
%{_bindir}/MistInRTSP
%{_bindir}/MistInSDP
%{_bindir}/MistInSubRip
%{_bindir}/MistInTS
%{_bindir}/MistInTSRIST
%{_bindir}/MistInTSSRT
%{_bindir}/MistOutAAC
%{_bindir}/MistOutCMAF
%{_bindir}/MistOutDTSC
%{_bindir}/MistOutEBML
%{_bindir}/MistOutFLV
%{_bindir}/MistOutH264
%{_bindir}/MistOutHDS
%{_bindir}/MistOutHLS
%{_bindir}/MistOutHTTP
%{_bindir}/MistOutHTTPMinimalServer
%{_bindir}/MistOutHTTPS
%{_bindir}/MistOutHTTPTS
%{_bindir}/MistOutJPG
%{_bindir}/MistOutJSON
%{_bindir}/MistOutMP3
%{_bindir}/MistOutMP4
%{_bindir}/MistOutOGG
%{_bindir}/MistOutRTMP
%{_bindir}/MistOutRTSP
%{_bindir}/MistOutSDP
%{_bindir}/MistOutSubRip
%{_bindir}/MistOutTS
%{_bindir}/MistOutTSSRT
%{_bindir}/MistOutTSRIST
%{_bindir}/MistOutWAV
%{_bindir}/MistOutWebRTC
%{_bindir}/MistProcFFMPEG
%{_bindir}/MistProcLivepeer
%{_bindir}/MistProcMKVExec
%{_bindir}/MistSession
%{_bindir}/MistTranslateH264
%{_bindir}/MistUtilAMF
%{_bindir}/MistUtilCertbot
%{_bindir}/MistUtilMETA
%{_bindir}/MistUtilNuke
%{_bindir}/MistUtilRAX
%{_bindir}/MistAnalyserFLAC
%{_bindir}/MistInFLAC
%{_bindir}/MistOutFLAC
%{_bindir}/MistOutJSONLine

%{_libdir}/libmist.so
%{_libdir}/libmist_srt.so
%{_unitdir}/mistserver.service

%license /usr/share/licenses/mistserver/UNLICENSE
%license /usr/share/licenses/mistserver/COPYING.md

%package devel
Summary: MistServer library headers
%description devel
MistServer library headers
Requires: mistserver
%files devel
%{_includedir}/mist/*.h

%package in-av
Summary: MistServer libav Input
%description in-av
MistServer libav Input
Requires: mistserver
%files in-av
%{_bindir}/MistInAV

%changelog
* Fri Nov 10 2023 Dimitri Tarassenko <mitka@mitka.us> 3.2.3
- Merge from DDVTECH development branch
* Tue Apr 04 2023 Dimitri Tarassenko <mitka@mitka.us> 3.2-x
- Updated for MistServer 3.2
* Mon Dec 26 2022 Dimitri Tarassenko <mitka@mitka.us> 1.0-1
- First stab at it
