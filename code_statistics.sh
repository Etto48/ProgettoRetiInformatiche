#!/bin/bash
HEADERS=$(find ./ -name "*.h")
CFILES=$(find ./ -name "*.c")
FILES=$HEADERS\ $CFILES

HEADERS_COUNT=$(echo $HEADERS | wc -w)
CFILES_COUNT=$(echo $CFILES | wc -w)

TEXT_LINES=$(cat $FILES | wc -l)
TEXT_WORDS=$(cat $FILES | wc -w)
TEXT_LETTERS=$(cat $FILES | wc -c)
CODE_LINES=$(cat $FILES | egrep -v '(^(\/\/|\s\*|\/\*).*|.*\*\/$)' | wc -l)
CODE_WORDS=$(cat $FILES | egrep -v '(^(\/\/|\s\*|\/\*).*|.*\*\/$)' | wc -w)
CODE_LETTERS=$(cat $FILES | egrep -v '(^(\/\/|\s\*|\/\*).*|.*\*\/$)' | wc -c)
COMMENTS_LINES=$(cat $FILES | egrep '(^(\/\/|\s\*|\/\*).*|.*\*\/$)' | wc -l)
COMMENTS_WORDS=$(cat $FILES | egrep '(^(\/\/|\s\*|\/\*).*|.*\*\/$)' | wc -w)
COMMENTS_LETTERS=$(cat $FILES | egrep '(^(\/\/|\s\*|\/\*).*|.*\*\/$)' | wc -c)

let COM_TO_COD=$COMMENTS_LETTERS*100/$CODE_LETTERS

if [[ $COM_TO_COD -le "5" ]]; then
    COM_RATING="Horrible"
elif [[ $COM_TO_COD -le "10" ]]; then
    COM_RATING="Poor\t"
elif [[ $COM_TO_COD -le "15" ]]; then
    COM_RATING="Average\t"
elif [[ $COM_TO_COD -le "25" ]]; then
    COM_RATING="Good\t"
elif [[ $COM_TO_COD -gt "25" ]]; then
    COM_RATING="Excellent"
fi

echo -e "+-------------------------------------------------------+"
echo -e "| Statistics                                            |"
echo -e "+----------+------------+---------------+---------------+"
echo -e "| Type     | Lines      | Words         | Letters       |" 
echo -e "+----------+------------+---------------+---------------+"
echo -e "| Total    |\t$TEXT_LINES\t|\t$TEXT_WORDS\t|\t$TEXT_LETTERS\t|"
echo -e "| Code     |\t$CODE_LINES\t|\t$CODE_WORDS\t|\t$CODE_LETTERS\t|"
echo -e "| Comments |\t$COMMENTS_LINES\t|\t$COMMENTS_WORDS\t|\t$COMMENTS_LETTERS\t|"
echo -e "+----------+------------+---------------+---------------+"
echo -e "| File Type             | File count                    |"
echo -e "+-----------------------+-------------------------------+"
echo -e "| .h files              |\t$HEADERS_COUNT\t                |"
echo -e "| .c files              |\t$CFILES_COUNT\t                |"
echo -e "+-----------------------+-------------------------------+"
echo -e "| Comment to code ratio | Comment ratio rating          |"
echo -e "|\t$COM_TO_COD%\t        |\t$COM_RATING\t        |"
echo -e "+-----------------------+-------------------------------+"