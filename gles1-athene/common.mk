#
# Copyright  2010, QNX Software Systems Ltd.  All Rights Reserved
#
# This source code has been published by QNX Software Systems Ltd.
# (QSSL).  However, any use, reproduction, modification, distribution
# or transfer of this software, or any software which includes or is
# based upon any of this code, is only permitted under the terms of
# the QNX Open Community License version 1.0 (see licensing.qnx.com for
# details) or as otherwise expressly authorized by a written license
# agreement from QSSL.  For more information, please email licensing@qnx.com.
#
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=EGL demo application
endef

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

include $(MKFILES_ROOT)/qmacros.mk

include $(MKFILES_ROOT)/qtargets.mk

#===== LIBS - a space-separated list of library items to be included in the link.
ifeq ($(CPU), arm)
	ifneq ($(filter v7, $(VARIANT_LIST)), v7)
		GCCVER:= $(if $(GCC_VERSION), $(GCC_VERSION), $(shell qcc -V 2>&1 | grep default | sed -e 's/,.*//'))
		ifneq ($(filter 4.%, $(strip $(GCCVER))),)
			CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
			LIBS += m-vfp
		else
			LIBS += m
		endif
	else
		LIBS += m
	endif
else
	LIBS += m
endif

LIBS += EGL GLESv1_CM screen
