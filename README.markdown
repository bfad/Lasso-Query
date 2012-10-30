Query
=====

Query is a custom type designed to allow for database interaction in an object-oriented manner.


Installation
------------

### Pre-compiled Libraries

1. Click the big "Downloads" button next to the description on this page.
2. Choose the proper download for your platform
3. Decompress the file and move it into `$LASSO9_HOME/LassoLibraries/`

### Compile From Source

    $> cd where/you/want/the/source/installed/
    $> git clone https://github.com/bfad/Lasso-Query.git
    $> cd Lasso-Query
    $> make
    $> make install

_Note: If you're compiling on Mac OS X, you'll need the 10.5 SDK installed. You can follow the instructions [here](http://hints.macworld.com/article.php?story=20110318050811544) to restore the 10.5 SDK to Xcode 4._

### Install the Source File

1. Download the query.inc file you want from the raw link. (Latest development file should be https://raw.github.com/bfad/Lasso-Query/master/query.inc)
2. Move the downloaded file into `$LASSO9_HOME/LassoStartup/`


Examples Using Inline Parameters
--------------------------------

Below are examples of how to use query as a straight replacement for the [inline] method. Each of the examples below assume that you have connection to a database named "rhino" with a table named "people" with the following fields:

* name\_preferred
* name\_first 
* name\_middle
* name\_last
* type
* email

### Basic Example

In this example, we will find all the people whose `name_last` starts with the letter "A" and stick their last name into an array.

    local(a_last_names) = array
    query(-database='rhino', -table='people', -search, -operator='bw', 'name_last'='A')->forEach => {
        #a_last_names->insert(#1->name_last)
    }
    
### Example with HTML

Here we're displaying various fields in an HTML table for everyone with a last name starting with the letter "A". Notice that the last column uses the `[query_result->c()]` method to reference `type` &mdash; this is because there is already a built-in method named `type` to every Lasso type.

    <?=    
        local(people) = query(
            -database="rhino",
                 -sql="SELECT p.name_preferred, p.name_first, p.name_middle, 
                         p.name_last, p.email, p.type
                      FROM people p WHERE p.name_last LIKE 'A%'"
        )
    ?>
    <table>
    [with person in #people do {^]    
        <tr>
            <td>[#person->name_first]</td>
            <td>[#person->name_middle]</td>
            <td>[#person->name_last]</td>
            <td>[#person->c('type')]</td>
        </tr>
    [^}]
    </table>
    
### Example With Custom [trait\_query\_result] Type

In this example we are using a custom `[trait_query_result]` type to add some additional member methods, and then use those methods while displaying in an HTML table everyone whose name begins with the letter "A".

    <?=
        define person => type {
            trait { import trait_query_result }
            data protected __cols,
                 protected __data
    
            public onCreate(cols::trait_positionallyKeyed, data::trait_positionallyKeyed) => {
                .'__cols' = #cols
                .'__data' = #data
            }
    
            public .name_preferred => {
                return (.'name_preferred' == ''? .name_first | .'name_preferred')
            }
    
            public name_fml => .name_first + ' ' + .name_middle + ' ' + .name_last
            public name_pl  => .name_preferred + ' ' + .name_last
        }
        
        local(people) = query(
            'person',
            -database="rhino",
                 -sql="SELECT p.name_preferred, p.name_first, p.name_middle, p.name_last, p.email
                      FROM people p WHERE p.name_last LIKE 'A%' LIMIT 100"
        )
    ?>
    <table>
    [with person in #people do {^]    
        <tr>
            <td>[#person->name_pl]</td>
            <td>[#person->email]</td>
        </tr>
    [^}]
    </table>


Examples Using New Datasource Connection
----------------------------------------

The following examples will walk you through using the new \[ds\_connect\] and \[ds\_database\] types - each of which as a "query" member method that returns a \[query\] object. They will assume a similar database and table setup as the examples above.

###Basic \[ds\_connect\] Example

In this example, we'll specify all the necessary information needed to connect to a database and then run a small query displaying the connection id.

    local(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='marty', -password='thisIsHeavy')
    #conn->query("SELECT CONNECTION_ID() AS conn_id")->first->conn_id
    
###Using \[ds\_connect\] With Stored Connection Data

Here we will run through creating a ds\_connect object using information stored in Lasso 9's database registry. For this to work, a database with a hostname of 'localhost' and a username of 'doc' need to have been created in the Lasso 9 admin interface. After instantiating the object, will gather up all the last names in an array.

    local(last_names) = array
    local(conn)       = ds_connect('localhost', -username='doc')
    handle => { #conn->close }
    
    #conn->query("SELECT p.name_last FROM rhino.people p LIMIT 50")->forEach => {
        #last_names->insert(#1->name_last)
    }

###Sharing a Connection with Two \[ds\_database\] Objects

This example will show you how to use \[ds\_database\] and how multiple \[ds\_database\] objects can share a single \[ds\_connect\] connection. Note that the connection to the database by a \[ds\_connect\] object is only opened lazily - only when a query is actually performed. However, form that point it stays open until explicitly closed (or the lasso process quits, forcing a close). So each of these queries is executed over the same connection.

    <?=
        // The database method returns a ds_database object
        // This is the preferred method of creating such an object
        local(conn) = ds_connect('localhost', -username='doc')
        local(db1)  = #conn->database('rhino')
        local(db2)  = #conn->database('information_schema')
        
        handle => { #conn->close }
    ?>
    <table>
    [with char_set in #db2->query("SELECT * FROM CHARACTER_SETS") do {^]
        <tr><td>[#char_set->CHARACTER_SET_NAME]</td>
            <td>[#char_set->DESCRIPTION]</td>
        </tr>
    [^}]
    </table>
    <table>
    [with person in #db1->query("SELECT * FROM people") do {^]
        <tr><td>[#person->name_first]</td>
            <td>[#person->name_preferred]</td>
            <td>[#person->name_last]</td>
        </tr>
    [^}]
    </table>

###Example Using Custom [trait\_query\_result] Type

Both [ds\_connect->query(...)] and [ds\_database->query(...)] can also take two arguments - the first one specifying the custom types to use instead of [query\_result] and the second argument being the SQL statement.

    <?=
        define person => type {
            trait { import trait_query_result }
            data protected __cols,
                 protected __data
    
            public onCreate(cols::trait_positionallyKeyed, data::trait_positionallyKeyed) => {
                .'__cols' = #cols
                .'__data' = #data
            }
    
            public .name_preferred => {
                return (.'name_preferred' == ''? .name_first | .'name_preferred')
            }
    
            public name_fml => .name_first + ' ' + .name_middle + ' ' + .name_last
            public name_pl  => .name_preferred + ' ' + .name_last
        }
        local(conn) = ds_connect('localhost', -username='doc')
        handle => { #conn->close }
        
        local(people) = #conn->database('rhino')->query('person',
            "SELECT p.name_preferred, p.name_first, p.name_middle, p.name_last, p.email
             FROM people p WHERE p.name_last LIKE 'A%' LIMIT 100"
        )
    ?>
    <table>
    [with person in #people do {^]    
        <tr>
            <td>[#person->name_pl]</td>
            <td>[#person->email]</td>
        </tr>
    [^}]
    </table>


License
-------

Copyright 2012 Bradley Lindsay

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

>    [http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.