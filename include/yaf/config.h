/* include/yaf/config.h.  Generated from config.h.in by configure.  */
/* include/yaf/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if pointers to integers require aligned access */
/* #undef HAVE_ALIGNED_ACCESS_REQUIRED */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the <glob.h> header file. */
#define HAVE_GLOB_H 1

/* Define to 1 if you have the <grp.h> header file. */
#define HAVE_GRP_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `pcap' library (-lpcap). */
#define HAVE_LIBPCAP 1

/* Define to 1 if you have the `wpcap' library (-lwpcap). */
/* #undef HAVE_LIBWPCAP */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <pcap.h> header file. */
#define HAVE_PCAP_H 1

/* Define to 1 if you have the <pcap-zcopy.h> header file. */
/* #undef HAVE_PCAP_ZCOPY_H */

/* Define to 1 if you have the <pthread.h> header file. */
/* #undef HAVE_PTHREAD_H */

/* Define to 1 if you have the <pwd.h> header file. */
#define HAVE_PWD_H 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the <sp.h> header file. */
/* #undef HAVE_SP_H */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/errno.h> header file. */
#define HAVE_SYS_ERRNO_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 on Linux for privilege drop hack */
/* #undef LINUX_PRIVHACK */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "qof"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "brian@trammell.ch"

/* Define to the full name of this package. */
#define PACKAGE_NAME "qof"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "qof 0.9.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "qof"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.9.0"

/* size_t cast for string formatting */
#define SIZE_T_CAST uint64_t

/* (f)printf format string for type size_t */
#define SIZE_T_FORMAT PRIu64

/* (f)printf format string for type size_t */
#define SIZE_T_FORMATX PRIx64

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 to enable use of GOption */
#define USE_GOPTION 1

/* Define to 1 to use libpopt */
/* #undef USE_POPT */

/* Version number of package */
#define VERSION "0.9.0"

/* configure script options */
#define YAF_ACONF_STRING_STR "build configuration: glib opt processing|fixbuf 1.2.0 -L/opt/local/lib|compact IPv4|\n"

/* Define to 1 to enable BIVIO support */
/* #undef YAF_ENABLE_BIVIO */

/* Define to 1 to compact IPv4 addresses where possible */
#define YAF_ENABLE_COMPACT_IP4 1

/* Define to 1 to enable DAG support */
/* #undef YAF_ENABLE_DAG */

/* Define to 1 to enable recording DAG interface numbers */
/* #undef YAF_ENABLE_DAG_SEPARATE_INTERFACES */

/* Define to 1 to enable export of DNS Authoritative Responses Only */
/* #undef YAF_ENABLE_DNSAUTH */

/* Define to 1 to enable export of DNS NXDomain Responses Only */
/* #undef YAF_ENABLE_DNSNXDOMAIN */

/* Define to 1 to enable plugin support */
/* #undef YAF_ENABLE_HOOKS */

/* Define to 1 to enable Napatech support */
/* #undef YAF_ENABLE_NAPATECH */

/* Define to 1 to enable recording DAG interface numbers */
/* #undef YAF_ENABLE_NAPATECH_SEPARATE_INTERFACES */
