
##############################################################
#
# AESDI2C-ASSIGNMENTS
#
##############################################################

#TODO: Fill up the contents below in order to reference your assignment 3 git contents
AESDI2C_VERSION = 4ba95311e91ae8541209facde5a3e30abd8fffd1


# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
AESDI2C_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
AESDI2C_SITE_METHOD = git
AESDI2C_MODULE_SUBDIRS = aesd-i2c-driver/

$(eval $(kernel-module))
$(eval $(generic-package))
