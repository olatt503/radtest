#include "common.h"

DB *dbp;

void
db_open(char *filename, int flags)
{
        int ret;

        if (dbp) {
                die("%s", "database was already open");
        }

        ret = db_create(&dbp, NULL, 0);
        if (ret) {
                die("%s", "db_create");
        }

        ret = dbp->open(dbp, NULL, filename, NULL, DB_BTREE, flags, 0600);
        if (ret) {
                die("can't open database '%s'", filename);
        }
}

void
db_put(char *key, char *value)
{
        int ret;
        DBT key_dbt, data_dbt;

        memset(&key_dbt, 0, sizeof(key_dbt));
        memset(&data_dbt, 0, sizeof(data_dbt));

        key_dbt.flags = DB_DBT_USERMEM;
        key_dbt.data = key;
        key_dbt.size = strlen(key) + 1;

        data_dbt.data = value;
        data_dbt.size = strlen(value) + 1;

        ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, 0);
        if (ret) {
                die("dbp->put for key '%s' and value '%s' failed", key, value);
        }
}

char *
db_fetch (char *key)
{
        int ret;
        DBT key_dbt, data_dbt;

        memset(&key_dbt, 0, sizeof(key_dbt));
        memset(&data_dbt, 0, sizeof(data_dbt));

        key_dbt.flags = DB_DBT_USERMEM;
        key_dbt.data = key;
        key_dbt.size = strlen(key) + 1;

        data_dbt.flags = DB_DBT_MALLOC;

        ret = dbp->get(dbp, NULL, &key_dbt, &data_dbt, 0);
        if (ret) {
                return NULL;
        }

        return data_dbt.data;
}

void
db_close(void)
{
        int ret;

        if (dbp) {
                ret = dbp->close(dbp, 0);
                if (ret) {
                        die("%s", "dbp->close failed");
                }
                dbp = NULL;
        }
}

void
create_berkley_db(char *file)
{
        int ret, i;
        FILE *input;
        char buf[BUFSIZ];
        char key[BUFSIZ];
        char value[BUFSIZ];

        input = fopen(file, "r");

        if (! input) {
                die ("error opening %s", file);
        }

        db_open(DATABASE, DB_CREATE);

        while (fgets(buf, BUFSIZ - 1, input) != NULL) {
                ret = sscanf(buf, "%s\t%s", key, value);
                for (i = 0; key[i]; i++) {
                        key[i] = tolower(key[i]);
                }
                for (i = 0; value[i]; i++) {
                        value[i] = tolower(value[i]);
                }
                if (ret != 2) {
                        die("Could not parse line: '%s'. Aborting.", buf);
                }
                dprintf("Adding key = '%s' => value = '%s'\n", key, value);
                db_put(key, value);
        }

        db_close();
}
