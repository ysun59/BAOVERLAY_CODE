/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *
 * Copyright (C) 2011 Novell Inc.
 * Copyright (C) 2016 Red Hat, Inc.
 */

// hui start
#define BITMAPPARTITIONS	1024
#define PARTITIONSIZE		1048576
#define BLOCKSIZE 		4096 //this is how the underlying file is divided
#define BLOCKSTATE_LOWER	0
#define BLOCKSTATE_UPPER	1
#define	BLOCKSTATE_INTRANSIT	2
#define BLOCKBEFOREPROCESS	0
#define BLOCKINPROCESS		1
#define BLOCKPOSTPROCESS	2
extern struct list_head cow_work_queue;
extern struct hlist_head cow_work_table[1 << 20];
int is_upper(struct dentry*, loff_t);
int update_bitmap(struct dentry*, loff_t, int);

struct ovl_cow_work_t {
        struct dentry *dentry;   //ovl inode
	loff_t blockid;
        loff_t offset;
        ssize_t len;
        struct hlist_node worker_list;

	// syncup per block
	int lock;
};
void do_one_work(struct ovl_cow_work_t *work);
//hui end


struct ovl_config {
	char *lowerdir;
	char *upperdir;
	char *workdir;
	bool default_permissions;
	bool redirect_dir;
	bool redirect_follow;
	const char *redirect_mode;
	bool index;
	bool nfs_export;
	int xino;
	bool metacopy;
};

struct ovl_sb {
	struct super_block *sb;
	dev_t pseudo_dev;
};

struct ovl_layer {
	struct vfsmount *mnt;
	/* Trap in ovl inode cache */
	struct inode *trap;
	struct ovl_sb *fs;
	/* Index of this layer in fs root (upper idx == 0) */
	int idx;
	/* One fsid per unique underlying sb (upper fsid == 0) */
	int fsid;
};

struct ovl_path {
	struct ovl_layer *layer;
	struct dentry *dentry;
};

/* private information held for overlayfs's superblock */
struct ovl_fs {
	struct vfsmount *upper_mnt;
	unsigned int numlower;
	/* Number of unique lower sb that differ from upper sb */
	unsigned int numlowerfs;
	struct ovl_layer *lower_layers;
	struct ovl_sb *lower_fs;
	/* workbasedir is the path at workdir= mount option */
	struct dentry *workbasedir;
	/* workdir is the 'work' directory under workbasedir */
	struct dentry *workdir;
	/* index directory listing overlay inodes by origin file handle */
	struct dentry *indexdir;
	long namelen;
	/* pathnames of lower and upper dirs, for show_options */
	struct ovl_config config;
	/* creds of process who forced instantiation of super block */
	const struct cred *creator_cred;
	bool tmpfile;
	bool noxattr;
	/* Did we take the inuse lock? */
	bool upperdir_locked;
	bool workdir_locked;
	/* Traps in ovl inode cache */
	struct inode *upperdir_trap;
	struct inode *workdir_trap;
	struct inode *indexdir_trap;
	/* Inode numbers in all layers do not use the high xino_bits */
	unsigned int xino_bits;
};

/* private information held for every overlayfs dentry */
struct ovl_entry {
	union {
		struct {
			unsigned long flags;
		};
		struct rcu_head rcu;
	};
	unsigned numlower;
	struct ovl_path lowerstack[];
};

struct ovl_entry *ovl_alloc_entry(unsigned int numlower);

static inline struct ovl_entry *OVL_E(struct dentry *dentry)
{
	return (struct ovl_entry *) dentry->d_fsdata;
}

struct ovl_inode {
	union {
		struct ovl_dir_cache *cache;	/* directory */
		struct inode *lowerdata;	/* regular file */
	};
	const char *redirect;
	u64 version;
	unsigned long flags;
	struct inode vfs_inode;
	struct dentry *__upperdentry;
	struct inode *lower;

	// hui: start lazy copy-on-write
	struct file	*meta;
	char		*bitmap[BITMAPPARTITIONS];  
	//char		bitmap_temp[256];
        struct file 	*lower_file;		/* points to lower file */
        struct file 	*upper_file;		/* points to upper file */
	int 		cow_status;   		/* 1 means in-cow, while 0 means normal */
	loff_t		lowerfile_last_block;	/* stores size of the lower file */
	loff_t		lowerfile_len;
	// hui end

	/* synchronize copy up and more */
	struct mutex lock;
};

static inline struct ovl_inode *OVL_I(struct inode *inode)
{
	return container_of(inode, struct ovl_inode, vfs_inode);
}

static inline struct dentry *ovl_upperdentry_dereference(struct ovl_inode *oi)
{
	return READ_ONCE(oi->__upperdentry);
}
