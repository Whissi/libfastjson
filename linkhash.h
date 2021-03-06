/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _fj_linkhash_h_
#define _fj_linkhash_h_

#include "json_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * golden prime used in hash functions
 * internal functionality - NOT PART OF THE API
 */
#define LH_PRIME 0x9e370001UL

/**
 * The fraction of filled hash buckets until an insert will cause the table
 * to be resized.
 * This can range from just above 0 up to 1.0.
 * internal functionality - NOT PART OF THE API
 */
#define LH_LOAD_FACTOR 0.66

/**
 * sentinel pointer value for empty slots
 * internal functionality - NOT PART OF THE API
 */
#define LH_EMPTY (void*)-1

/**
 * sentinel pointer value for freed slots
 * internal functionality - NOT PART OF THE API
 */
#define LH_FREED (void*)-2

/**
 * default string hash function
 */
#define FJSON_STR_HASH_DFLT 0

/**
 * perl-like string hash function
 */
#define FJSON_STR_HASH_PERLLIKE 1

/**
 * This function sets the hash function to be used for strings.
 * Must be one of the FJSON_STR_HASH_* values.
 * @returns 0 - ok, -1 if parameter was invalid
 */
int fjson_global_set_string_hash(const int h);

struct lh_entry;

/**
 * callback function prototypes
 */
typedef void (lh_entry_free_fn) (struct lh_entry *e);
/**
 * callback function prototypes
 */
typedef unsigned long (lh_hash_fn) (const void *k);
/**
 * callback function prototypes
 */
typedef int (lh_equal_fn) (const void *k1, const void *k2);

/**
 * An entry in the hash table
 * internal functionality - NOT PART OF THE API
 */
struct lh_entry {
	/**
	 * The key.
	 */
	void *k;
	int k_is_constant;
	/**
	 * The value.
	 */
	const void *v;
	/**
	 * The next entry
	 */
	struct lh_entry *next;
	/**
	 * The previous entry.
	 */
	struct lh_entry *prev;
};


/**
 * The hash table structure.
 * internal functionality - NOT PART OF THE API
 */
struct lh_table {
	/**
	 * Size of our hash.
	 */
	int size;
	/**
	 * Numbers of entries.
	 */
	int count;

	/**
	 * The first entry.
	 */
	struct lh_entry *head;

	/**
	 * The last entry.
	 */
	struct lh_entry *tail;

	struct lh_entry *table;

	/**
	 * A pointer onto the function responsible for freeing an entry.
	 */
	lh_entry_free_fn *free_fn;
	lh_hash_fn *hash_fn;
	lh_equal_fn *equal_fn;

	/* we set aside memory for the default table size, so
	 * as long as we do not have more entries than these, we
	 * do not need to malloc the table. This is at the price
	 * of wasted memory for tables with MORE entries.
	 */
	struct lh_entry dflt_table[FJSON_OBJECT_DEF_HASH_ENTRIES];
};


/**
 * Convenience list iterator.
 * internal functionality - NOT PART OF THE API
 */
#define lh_foreach(table, entry) \
for(entry = table->head; entry; entry = entry->next)

/**
 * lh_foreach_safe allows calling of deletion routine while iterating.
 * internal functionality - NOT PART OF THE API
 */
#define lh_foreach_safe(table, entry, tmp) \
for(entry = table->head; entry && ((tmp = entry->next) || 1); entry = tmp)



/**
 * Create a new linkhash table.
 * @param size initial table size. The table is automatically resized
 * although this incurs a performance penalty.
 * @param free_fn callback function used to free memory for entries
 * when lh_table_free or lh_table_delete is called.
 * If NULL is provided, then memory for keys and values
 * must be freed by the caller.
 * @param hash_fn  function used to hash keys. 2 standard ones are defined:
 * lh_ptr_hash and lh_char_hash for hashing pointer values
 * and C strings respectively.
 * @param equal_fn comparison function to compare keys. 2 standard ones defined:
 * lh_ptr_hash and lh_char_hash for comparing pointer values
 * and C strings respectively.
 * @return a pointer onto the linkhash table.
 * internal functionality - NOT PART OF THE API
 */
extern struct lh_table* lh_table_new(int size,
				     lh_entry_free_fn *free_fn,
				     lh_hash_fn *hash_fn,
				     lh_equal_fn *equal_fn);

/**
 * Convenience function to create a new linkhash
 * table with char keys.
 * @param size initial table size.
 * @param name table name.
 * @param free_fn callback function used to free memory for entries.
 * @return a pointer onto the linkhash table.
 * internal functionality - NOT PART OF THE API
 */
extern struct lh_table* lh_kchar_table_new(int size,
					   lh_entry_free_fn *free_fn);


/**
 * Convenience function to create a new linkhash
 * table with ptr keys.
 * @param size initial table size.
 * @param name table name.
 * @param free_fn callback function used to free memory for entries.
 * @return a pointer onto the linkhash table.
 * internal functionality - NOT PART OF THE API
 */
extern struct lh_table* lh_kptr_table_new(int size,
					  lh_entry_free_fn *free_fn);


/**
 * Free a linkhash table.
 * If a callback free function is provided then it is called for all
 * entries in the table.
 * @param t table to free.
 * internal functionality - NOT PART OF THE API
 */
extern void lh_table_free(struct lh_table *t);


/**
 * Insert a record into the table.
 * @param t the table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 * internal functionality - NOT PART OF THE API
 */
extern int lh_table_insert(struct lh_table *t, void *k, const void *v);


/**
 * Insert a record into the table. This one accepts the key's hash in additon
 * to the key. This is an exension to support functions that need to calculate
 * the hash several times and allows them to do it just once and then pass
 * in the hash to all utility functions. Depending on use case, this can be a 
 * very considerate performance improvement.
 * @param t the table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 * @param h hash value of the key to insert
 * @param opts opts, a subset of FJSON_OBJECT_ADD_* flags is supported
 * internal functionality - NOT PART OF THE API
 */
extern int lh_table_insert_w_hash(struct lh_table *t, void *k, const void *v, const unsigned long h, const unsigned opts);


/**
 * Lookup a record into the table.
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 * internal functionality - NOT PART OF THE API
 */
extern struct lh_entry* lh_table_lookup_entry(struct lh_table *t, const void *k);

/**
 * Lookup a record into the table. This one accepts the key's hash in additon
 * to the key. This is an exension to support functions that need to calculate
 * the hash several times and allows them to do it just once and then pass
 * in the hash to all utility functions. Depending on use case, this can be a 
 * very considerate performance improvement.
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @param h hash value of the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 * internal functionality - NOT PART OF THE API
 */
extern struct lh_entry* lh_table_lookup_entry_w_hash(struct lh_table *t, const void *k, const unsigned long h);

/**
 * Lookup a record into the table
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @return a pointer to the found value or NULL if it does not exist.
 * @deprecated Use lh_table_lookup_ex instead.
 * internal functionality - NOT PART OF THE API
 */
THIS_FUNCTION_IS_DEPRECATED(extern const void* lh_table_lookup(struct lh_table *t, const void *k));

/**
 * Lookup a record in the table
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @param v a pointer to a where to store the found value (set to NULL if it doesn't exist).
 * @return whether or not the key was found
 * internal functionality - NOT PART OF THE API
 */
extern fjson_bool lh_table_lookup_ex(struct lh_table *t, const void *k, void **v);

/**
 * Delete a record from the table.
 * If a callback free function is provided then it is called for the
 * for the item being deleted.
 * @param t the table to delete from.
 * @param e a pointer to the entry to delete.
 * @return 0 if the item was deleted.
 * @return -1 if it was not found.
 * internal functionality - NOT PART OF THE API
 */
extern int lh_table_delete_entry(struct lh_table *t, struct lh_entry *e);


/**
 * Delete a record from the table.
 * If a callback free function is provided then it is called for the
 * for the item being deleted.
 * @param t the table to delete from.
 * @param k a pointer to the key to delete.
 * @return 0 if the item was deleted.
 * @return -1 if it was not found.
 * internal functionality - NOT PART OF THE API
 */
extern int lh_table_delete(struct lh_table *t, const void *k);

extern int lh_table_length(struct lh_table *t);

void lh_abort(const char *msg, ...) __attribute__((__format__(__printf__, 1, 2)));
void lh_table_resize(struct lh_table *t, int new_size);


/**
 * Calculate the hash of a key for a given table.
 * This is an exension to support functions that need to calculate
 * the hash several times and allows them to do it just once and then pass
 * in the hash to all utility functions. Depending on use case, this can be a 
 * very considerate performance improvement.
 * @param t the table (used to obtain hash function)
 * @param k a pointer to the key to lookup
 * @return the key's hash
 */
static inline unsigned long lh_get_hash(const struct lh_table *t, const void *k)
{
	return t->hash_fn(k);
}

#ifndef FJSON_NATIVE_API_ONLY
#define JSON_C_STR_HASH_PERLLIKE FJSON_STR_HASH_PERLLIKE
#define json_global_set_string_hash fjson_global_set_string_hash
#endif


#ifdef __cplusplus
}
#endif

#endif
