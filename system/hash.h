#ifndef HASH_H
#define HASH_H

#ifndef NSEED
#define	NSEED	32	/* ���ɽ�μ��ࡣ2��沾�Ǥʤ���Фʤ�ʤ��� */
#endif

#ifndef NSIZE
#define NSIZE	256
#endif

#define	HASH_TBLSIZE	8192

#define	HASH_READ_MODE	1
#define	HASH_WRITE_MODE	2

#define	HASH_NOOVERWRITE	8
#define	HASH_KEYEXIST		(-30996)

/* �¥ǡ��� (����ɽ��) */
typedef struct _hash_data_unit_internal {
    unsigned int	key_size;
    unsigned int	value_size;
    char		*key;
    char		*value;
    struct _hash_data_unit_internal	*next;	/* conflict���˳�Ǽ���� */
} HASH_DATA_UNIT_INTERNAL;

typedef struct {
    unsigned int	pos;
} HASH_HEADER_UNIT;

typedef struct {
    unsigned int	key_size;
    unsigned int	value_size;
} HASH_DATA_UNIT;

typedef struct {
    FILE *fp;
    int mode;
    unsigned int seed[NSEED][NSIZE];
    HASH_HEADER_UNIT *hhdata;
    HASH_DATA_UNIT_INTERNAL **hddata;
} HASH_FILE;

#endif
