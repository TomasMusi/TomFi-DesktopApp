
import { createPool } from 'mysql2' // do not use 'mysql2/promises'!
import { Kysely, MysqlDialect } from 'kysely'
import type { DB } from './db_types.ts'
import 'dotenv/config';

const dialect = new MysqlDialect({
    //@ts-ignore
    pool: createPool({
        database: process.env.DATABASE_NAME,
        host: process.env.DATABASE_IP,
        user: process.env.DATABASE_USER,
        password: process.env.DATABASE_PASSWORD,
        port: parseInt(process.env.DATABASE_PORT || '3306')
    })
})

export const db = new Kysely<DB>({
    dialect,
})
