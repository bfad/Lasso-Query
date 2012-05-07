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


Examples
--------

Each of the examples below assume that you have connection to a database named "rhino" with a table named "people" with the following fields:

* name\_preferred
* name\_first 
* name\_middle
* name\_last
* type
* email

### Basic Example

In this example, we will find all the people whose `name_last` starts with the letter "A" and stick their last name into an array.

    local(a_last_names) = array
    query(-database='rhino', -table='people', -operator='bw', 'name_last'='A')->forEach => {
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
            data protected __cols::map
    
            public onCreate(cols::map) => {
                .'__cols' = #cols
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