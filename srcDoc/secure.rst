.. _section-secure:

Secure Certificates, Governance and Permission Files
====================================================

The performance test provides a set of already generated certificates,
governance and permission files to be loaded when using the RTI Secure
DDS plugin. Both governance files and permission files are already
signed, so no action is required by the user. These files are located in
``$(RTIPERFTESTHOME)/resource/secure``.

In addition to the already signed governance and permission files, the
original files are also provided (not signed) as well as a ``bash``
script with the steps to generate all the signed files. Those files can
be found in ``$(RTIPERFTESTHOME)/resource/secure/input``; the script is
in ``$(RTIPERFTESTHOME)/resource/secure/make.sh``.
