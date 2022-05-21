#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#define FINMLEN 128
#define MODELEN 12
#define INOALIGN 12
struct fileattrs {
	char f_modes[MODELEN];
	char f_name[FINMLEN];
	ino_t f_inode;
	uid_t f_uid;
	gid_t f_gid;
	off_t f_size;
};
DIR *Opendir(const char *);
int Lstat(const char *, struct stat *);
int getfilemodes(struct fileattrs *, struct stat *);
int getfileinfo(struct fileattrs *, struct stat *,
	struct passwd **, struct group **);
int main(int argc, char *argv[])
{
	char path[1024] = {'\0'};
	DIR *dirfp = NULL;
	struct dirent *dirptr = NULL;
	struct passwd *pd = NULL;
	struct group *grp = NULL;
	struct stat statbuf;
	struct fileattrs fattr;

	if (argc < 2) {
		if (!getcwd(path, 1024)) {
			perror("getcwd()");
			exit(EXIT_FAILURE);
		}
	} else {
		strncpy(path, argv[1], 1024);
	}
	Lstat(path, &statbuf);
	if (S_ISDIR(statbuf.st_mode)) {
		dirfp = Opendir(path);
		while ((dirptr = readdir(dirfp))) {
			memset(fattr.f_name, '\0', sizeof(char) * FINMLEN);
			memset(fattr.f_modes, '-', sizeof(char) * MODELEN - 2);
			fattr.f_modes[10] = fattr.f_modes[11] = '\0';
			strcpy(fattr.f_name, path);
			if (fattr.f_name[strlen(path) - 1] != '/')
				fattr.f_name[strlen(path)] = '/';
			strcat(fattr.f_name, dirptr->d_name);
			Lstat(fattr.f_name, &statbuf);
			getfileinfo(&fattr, &statbuf, &pd, &grp);
			printf("%*lu %s %s %s %*ld %s\n", INOALIGN, fattr.f_inode, fattr.f_modes,
				pd->pw_name, grp->gr_name, INOALIGN, fattr.f_size, dirptr->d_name);
		}
	} else {
		memset(fattr.f_modes, '-', sizeof(char) * MODELEN - 2);
		fattr.f_modes[10] = fattr.f_modes[11] = '\0';
		getfileinfo(&fattr, &statbuf, &pd, &grp);
		printf("%*lu %s %s %s %*ld %s\n", INOALIGN, fattr.f_inode, fattr.f_modes,
			pd->pw_name, grp->gr_name, INOALIGN, fattr.f_size, path);
	}
	closedir(dirfp);
	exit(EXIT_SUCCESS);
}
DIR *Opendir(const char *name)
{
	DIR *ret = NULL;
	if (!(ret = opendir(name))) {
		perror("opendir()");
		exit(EXIT_FAILURE);
	}
	return ret;
}
int Lstat(const char *path, struct stat *statbuf)
{
	if (lstat(path, statbuf)) {
		perror("stat()");
		exit(EXIT_FAILURE);
	}
	return 0;
}
int getfilemodes(struct fileattrs *fattr, struct stat *statbuf)
{
	/* get file type. */
	if (S_ISREG(statbuf->st_mode))
		fattr->f_modes[0] = '-';
	if (S_ISDIR(statbuf->st_mode))
		fattr->f_modes[0] = 'd';
	if (S_ISCHR(statbuf->st_mode))
		fattr->f_modes[0] = 'c';
	if (S_ISBLK(statbuf->st_mode))
		fattr->f_modes[0] = 'b';
	if (S_ISLNK(statbuf->st_mode))
		fattr->f_modes[0] = 'l';
	if (S_ISFIFO(statbuf->st_mode))
		fattr->f_modes[0] = 'p';
	if (S_ISSOCK(statbuf->st_mode))
		fattr->f_modes[0] = 's';
	/* get file modes. */
	if ((statbuf->st_mode & S_IRUSR) == S_IRUSR)
		fattr->f_modes[1] = 'r';
	if ((statbuf->st_mode & S_IWUSR) == S_IWUSR)
		fattr->f_modes[2] = 'w';
	if ((statbuf->st_mode & S_IXUSR) == S_IXUSR)
		fattr->f_modes[3] = 'x';
	if ((statbuf->st_mode & S_IRGRP) == S_IRGRP)
		fattr->f_modes[4] = 'r';
	if ((statbuf->st_mode & S_IWGRP) == S_IWGRP)
		fattr->f_modes[5] = 'w';
	if ((statbuf->st_mode & S_IXGRP) == S_IXGRP)
		fattr->f_modes[6] = 'x';
	if ((statbuf->st_mode & S_IROTH) == S_IROTH)
		fattr->f_modes[7] = 'r';
	if ((statbuf->st_mode & S_IWOTH) == S_IWOTH)
		fattr->f_modes[8] = 'w';
	if ((statbuf->st_mode & S_IXOTH) == S_IXOTH)
		fattr->f_modes[9] = 'x';
	if ((statbuf->st_mode & S_ISUID) == S_ISUID)
		fattr->f_modes[3] = 's';
	if ((statbuf->st_mode & S_ISGID) == S_ISGID)
		fattr->f_modes[6] = 's';
	if ((statbuf->st_mode & S_ISVTX) == S_ISVTX)
		fattr->f_modes[9] = 't';
	return 0;
}
int getfileinfo(struct fileattrs *fattr, struct stat *statbuf,
	struct passwd **pd, struct group **grp)
{
	getfilemodes(fattr, statbuf);
	fattr->f_inode = statbuf->st_ino;
	fattr->f_uid = statbuf->st_uid;
	fattr->f_gid = statbuf->st_gid;
	fattr->f_size = statbuf->st_size;
	*pd = getpwuid(fattr->f_uid);
	*grp = getgrgid(fattr->f_gid);
	return 0;
}
