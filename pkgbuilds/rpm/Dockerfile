FROM fedora:39 AS buildcontainer

ARG BUILD_NUMBER=1
ARG PACKAGE=nbfc-linux
ARG VERSION=0.3.17
RUN dnf install -y autoconf libcurl-devel openssl-devel dh-autoreconf clang make systemd-devel fedora-packager rpmdevtools ca-certificates
RUN useradd rpmbuild -u 5002 -g users -p rpmbuild
USER rpmbuild
ENV HOME /home/rpmbuild
WORKDIR /home/rpmbuild
RUN mkdir -p /home/rpmbuild/{BUILD,RPMS,SPECS,SRPMS,SOURCES}
RUN echo '%_topdir %{getenv:HOME}' > /home/rpmbuild/.rpmmacros
# RUN git clone --depth 1 --branch 0.1.15 https://github.com/nbfc-linux/nbfc-linux.git SOURCES
COPY nbfc-linux.spec SPECS/
RUN pwd

RUN rpmbuild -ba --define "_buildnr ${BUILD_NUMBER}" --define "_myversion $VERSION" ./SPECS/${PACKAGE}.spec

FROM scratch as custom-exporter
COPY --from=buildcontainer /home/rpmbuild/RPMS/*  .

