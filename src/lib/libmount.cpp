#include<cerrno>
#include<cstring>
#include<format>
#include"libmounts.h"

std::string libmount_strerror(int err){
	if(err>=-4095)return strerror(err);
	switch(err){
		case MNT_ERR_NOFSTAB:return "Not found required entry in fstab";
		case MNT_ERR_NOFSTYPE:return "Failed to detect filesystem type";
		case MNT_ERR_NOSOURCE:return "Required mount source undefined";
		case MNT_ERR_LOOPDEV:return "Loopdev setup failed, errno set by libc";
		case MNT_ERR_MOUNTOPT:return "Failed to parse/use userspace mount options";
		case MNT_ERR_APPLYFLAGS:return "Failed to apply MS_PROPAGATION flags, and MOUNT_ATTR_* attributes for mount_setattr";
		case MNT_ERR_AMBIFS:return "Libblkid detected more filesystems on the device";
		case MNT_ERR_LOOPOVERLAP:return "Detected overlapping loop device that cannot be re-used";
		case MNT_ERR_LOCK:return "Failed to lock utab or so";
		case MNT_ERR_NAMESPACE:return "Failed to switch namespace";
		case MNT_ERR_ONLYONCE:return "Filesystem mounted, but --onlyonce specified";
		case MNT_ERR_CHOWN:return "Filesystem mounted, but subsequent X-mount.owner=/X-mount.group= lchown failed";
		case MNT_ERR_CHMOD:return "Filesystem mounted, but subsequent X-mount.mode= chmod failed";
		case MNT_ERR_IDMAP:return "Filesystem mounted, but subsequent X-mount.idmap= failed";
		default:return std::format("Unknown error ({})",err);
	}
}
