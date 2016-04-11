# AQL_Compiler
AQL Subset Compiler

1. Where to run
	
	The tiny compiler could be run on unix based os.

2. How to run

	'cd ./src' -> 'make clean' -> 'make' -> '.Aql ../dataset/arg1 ../dataser/arg2' 
	arg1 is the aql file to be executed.
	arg2 is the text file to be processed, single file and floder are both available.

	Eg:
	cd ./src
	make clean
	make
	./Aql ../dataset/PerLoc.aql ../dataset/perloc/PerLoc.input
	or	./Aql ../dataset/PerLoc.aql ../dataset/perloc/

3. Function

	AQL could create views by extract regex from document, extract pattern and select from existent views' columns. It also could output the views.

	The output will be displayed in terminal and output.txt, which is in src floder.
	
4. Error Handling
	
	The errors of aql file will be detected by Parser's matching process.
	The program will shut down and print the line and column numbers.

5. Grammar

	The original productions are as follows:

	aql_stmt → create_stmt ; | output_stmt ; 
	create_stmt → create view ID as view_stmt
	view_stmt → select_stmt | extract_stmt
	output_stmt → output view ID alias alias → as ID | ε
	select_stmt → select select_list from from_list 
	select_list → select_item | select_list , select_item 
	select_item → ID . ID alias
	from_list → from_item | from_list , from_item 
	from_item → ID ID
	extract_stmt → extract extract_spec from from_list 
	extract_spec → regex_spec | pattern_spec
	regex_spec → regex REG on column name_spec
	column → ID . ID
	name_spec → as ID | return group_spec
	group_spec → single_group | group_spec and single_group 
	single_group → group NUM as ID
	pattern_spec → pattern pattern_expr name_spec 
	pattern_expr → pattern_pkg | pattern_expr pattern_pkg 
	pattern_pkg → atom | atom { NUM , NUM } | pattern_group 
	atom→ < column > | < Token > | REG
	pattern_group → ( pattern_expr )

	*To eliminate left recursive productions, we changed them:

	---from:
	select_list → select_item | select_list , select_item
	---to:
	select_list → select_item select_other
	select_other → select_item select_other | ε

	---from:
	from_list → from_item | from_list , from_item
	---to:
	from_list → from_item from_other
	from_other → from_item from_other | ε

	---from:
	group_spec → single_group | group_spec and single_group
	---to:
	group_spec → single_group group_other
	group_other → single_group group_other | ε

	---from:
	pattern_expr → pattern_pkg | pattern_expr pattern_pkg 
	---to:
	pattern_expr → pattern_pkg pattern_expr_other
	pattern_other → pattern_pkg pattern_expr_other | ε

	*To make the implementations easier, we changed them:

	---from:
	extract_stmt → extract extract_spec from from_list
	extract_spec → regex_spec | pattern_spec
	regex_spec → regex REG on column name_spec
	pattern_spec → pattern pattern_expr name_spec
	---to:
	extract_stmt → extract extract_spec
	extract_spec → regex_spec | pattern_spec
	regex_spec → regex REG on column name_spec from from_list
	pattern_spec → pattern pattern_expr name_spec from from_list
