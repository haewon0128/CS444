#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <time.h>

void printStuff(struct stat *);
void printStuff(struct stat *sb)
{
	struct passwd * pwu;
	struct group * grp;
	int r,w,x, usum, gsum, osum = 0;
	struct tm *tmp;
	char MY_TIME[50];





	grp = getgrgid(sb->st_gid);
	pwu = getpwuid(sb->st_uid);
	printf(" Device ID number:	   ");
	printf("%lu\n",sb->st_dev);




	printf(" I-node number:		   ");
	printf("%lu\n",sb->st_ino);




	printf(" Mode:			   ");

	printf( (S_ISREG(sb->st_mode)) ? "-":"");
	printf( (S_ISDIR(sb->st_mode)) ? "d":"");
	printf( (S_ISLNK(sb->st_mode)) ? "l":"");
	printf( (S_ISCHR(sb->st_mode)) ? "c":"");
	printf( (S_ISBLK(sb->st_mode)) ? "b":"");
	printf( (S_ISFIFO(sb->st_mode)) ? "p":"");
	printf( (S_ISSOCK(sb->st_mode)) ? "s":"");

	printf( (sb->st_mode & S_IRUSR) ? "r" : "-");
	r = (sb->st_mode & S_IRUSR) ? 4 : 0;
	printf( (sb->st_mode & S_IWUSR) ? "w" : "-");
	w = (sb->st_mode & S_IWUSR) ?  2 : 0;
	printf( (sb->st_mode & S_IXUSR) ? "x" : "-");
	x = (sb->st_mode & S_IXUSR) ?  1 : 0;
	usum = r + w + x;

	printf( (sb->st_mode & S_IRGRP) ? "r" : "-");
	r = (sb->st_mode & S_IRGRP) ?  4 : 0;
	printf( (sb->st_mode & S_IWGRP) ? "w" : "-");
	w = (sb->st_mode & S_IWGRP) ?  2 : 0;
	printf( (sb->st_mode & S_IXGRP) ? "x" : "-");
	x = (sb->st_mode & S_IXGRP) ?  1 : 0;
	gsum = r + w + x;

	printf( (sb->st_mode & S_IROTH) ? "r" : "-");
	r = (sb->st_mode & S_IROTH) ?  4 : 0;
	printf( (sb->st_mode & S_IWOTH) ? "w" : "-");
	w = (sb->st_mode & S_IWOTH) ?  2 : 0;
	printf( (sb->st_mode & S_IXOTH) ? "x" : "-");
	x = (sb->st_mode & S_IXOTH) ?  1 : 0;
	osum = r + w + x;

	printf("		(%d%d%d in octal)\n",usum, gsum, osum);


	printf(" Link count:		   ");
	printf("%ld\n",sb->st_nlink);
	printf(" Owner Id:		   ");
	printf("%s", pwu->pw_name);
	printf("		(UID = %d)\n", sb->st_uid);



	printf(" Group Id:                 ");
	printf("%s", grp->gr_name);
	printf("              (GID = %d)\n", sb->st_gid);
	printf(" Preferred I/O block size: ");
	printf("%lu bytes\n", sb->st_blksize);
	printf(" File size:		   ");
	printf("%lu bytes\n", sb->st_size);
	printf(" Blocks allocated:	   ");
	printf("%lu\n", sb->st_blocks);



	tmp = localtime(&sb->st_atime);
	strftime(MY_TIME, sizeof(MY_TIME), "%Y-%m-%d %H:%M:%S %z (%Z) %a", tmp);
	printf(" Last file access:	   ");
	printf("%s (local)\n", MY_TIME);


	tmp = localtime(&sb->st_mtime);
	strftime(MY_TIME, sizeof(MY_TIME), "%Y-%m-%d %H:%M:%S %z (%Z) %a", tmp);
	printf(" Last file modification:   ");
	printf("%s (local)\n", MY_TIME);


	tmp = localtime(&sb->st_ctime);
	strftime(MY_TIME, sizeof(MY_TIME), "%Y-%m-%d %H:%M:%S %z (%Z) %a", tmp);
	printf(" Last status change:       ");
	printf("%s (local)\n", MY_TIME);
}

int main(int argc, char **argv)
{

	int i;
	int res;
	struct stat sb;
	struct stat nsb;
	ssize_t lr;

	if (argc < 2) 
	{
		fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	for(i = 1; i<argc; i++)
	{
		res = lstat(argv[i], &sb);
		if (res < 0) 
		{

			perror("stat");
			exit(EXIT_FAILURE);
		}

		else
		{	

			char *linkname = NULL;
			printf("File: %s\n", argv[i]);
			printf(" File type:                ");
			switch(sb.st_mode & S_IFMT){
				case S_IFREG: 
					printf("regular file\n");
					printStuff(&sb);
					break;
				case S_IFDIR:
					printf("directory\n");
					printStuff(&sb);
					break;
				case S_IFCHR:
					printf("character device\n");
					printStuff(&sb);
					break;
				case S_IFBLK:
					printf("block device\n");
					printStuff(&sb);
					break;
				case S_IFIFO:
					printf("FIFO/pipe\n");
					printStuff(&sb);
					break;
				case S_IFSOCK:
					printf("socket\n");
					printStuff(&sb);
					break;
				case S_IFLNK:
					if(stat(argv[i], &nsb) == -1)
					{
						printf("Symbolic link -> with dangling destination\n");
					}
					else
					{
						linkname = malloc(sb.st_size + 1);
						lr = readlink(argv[i], linkname, sb.st_size + 1);
						linkname[lr] = '\0';
						printf("Symbolic link -> %s\n", linkname);
						free(linkname);
					}
					printStuff(&sb);
					break; 
				default:
					printf("unknown?\n");
					break;
			}	
		}
	}
	return 0;
}




