#!/usr/bin/perl

use Cwd;

$saveoutput=0;
$nospaces=0;
$debug=0;

if ( $ENV{"DEBUG"} ) {
   $debug=$ENV{"DEBUG"};
}

if ( $ENV{"SAVEOUTPUT"} ) {
   $saveoutput=$ENV{"SAVEOUTPUT"};
}

if ( $ENV{"JUSTONETEST"} ) {
   $justonetest=$ENV{"JUSTONETEST"};
} else {
   $justonetest=0;
}

if ( $ENV{"DIFF"} ) {
   $diff=$ENV{"DIFF"};
} else {
   $diff="diff";
}

$HOME = $ENV{"HOME"};

#print "Home is $home\n";exit;

foreach $arg ( @ARGV ) {
    if ( $arg eq "-n" || $arg eq "--nospace" ) {
	$nospaces=1;
    } elsif ( $arg eq "-s" || $arg eq "--save" ) {
	$saveoutput = 1;
    } elsif ( $arg eq "-f" || $arg eq "--force" ) {
	$force = 1;
    } elsif ( $arg eq "-v" ) {
	$debug++;
    } else {
	print STDERR "Unknown option $arg\n";
	usage();
	exit(1);
    }
}

$libdir = $ENV{"lib"};
if ( $libdir eq "" ) {
    $libdir="$HOME/CalPoly/Class/cpe357/now/Asgn/Handin/lib/asgn1";
}
$username = $ENV{"name"};
if ( $username eq "" ) {
    $username = "Submitted";
}

$refdir = $ENV{"REFDIR"};
if ( $refdir eq "" ) {
    $refdir = "$HOME/CalPoly/Class/cpe357/now/Asgn/asgn1/Soln";
}
printf "DEBUG txlat: reference dir is $refdir\n" if ( $debug );

$inputdir = $ENV{"INPUTDIR"};
if ( $inputdir eq "" ) {
    $inputdir = "$libdir/Tests/Inputs";
}
printf "DEBUG tfs: image dir is $inputdir\n" if ( $debug );

$expdir = $ENV{"EXPDIR"};
if ( $expdir eq "" ) {
    $expdir = "$libdir/Tests/Expected";
}
printf "DEBUG tfs: expected dir is $expdir\n" if ( $debug );

# Force is only allowed if we are the owner of the expected output directory
if ( $force ) {
    $uid = getuid();
    $ownerid = (stat($expdir))[4];
    if ( $uid != $ownerid ) {
	print ERROR "force (-f) is only allowed for the owner of the Expected directory\n";
	print ERROR "      (un-setting)\n";
    }
    $force = 0;
}


if ( $saveoutput ) {
    if ( ! -d Output ) {
	mkdir "Output";
    }
}

%submitted = ( 
	       "xlat"    => "./xlat",
	       "minget"   => "./minget",
	       );

%reference = ( 
#	"xlat"   => "/bin/tr", # or /bin/tr
	"xlat"   => "$refdir/xlat", # or /bin/tr
	"minget"  => "$refdir/minget",
	       );

$maxlines = 10;
$maxchars = 2048;
$iolimit  = " iolimit $maxlines $maxchars ";
$timeout  = 5;
$sfile = "script.$$";

$notquotes="\001\002\003\004\005\006\007\010\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037!#&()*+,./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ^_abcdefghijklmnopqrstuvwxyz{|}~\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377";

$notquotesbackwards="\377\376\375\374\373\372\371\370\367\366\365\364\363\362\361\360\357\356\355\354\353\352\351\350\347\346\345\344\343\342\341\340\337\336\335\334\333\332\331\330\327\326\325\324\323\322\321\320\317\316\315\314\313\312\311\310\307\306\305\304\303\302\301\300\277\276\275\274\273\272\271\270\267\266\265\264\263\262\261\260\257\256\255\254\253\252\251\250\247\246\245\244\243\242\241\240\237\236\235\234\233\232\231\230\227\226\225\224\223\222\221\220\217\216\215\214\213\212\211\210\207\206\205\204\203\202\201\200\177~}|{zyxwvutsrqponmlkjihgfedcba_^ZYXWVUTSRQPONMLKJIHGFEDCBA?>=<;:9876543210/.,+*)(&#!\037\036\035\034\033\032\031\030\027\026\025\024\023\022\021\020\017\016\010\007\006\005\004\003\002\001";
#Each test consists of:
#             a name,  cmd, arguments, input, shouldfail, quiet
@tests = (
#--------------------------------------------------------------
#parsing errors    
[ "Usage:",                         	    "xlat",  "-h",                                                                        "/dev/null", 1,  0 ],
[ "Usage: too few args (trans)",    	    "xlat",  "abcde",                                                                     "/dev/null", 1,  0 ],
[ "Usage: too many args (trans)",   	    "xlat",  "-c abcde abcde extra",                                                      "/dev/null", 1,  0 ],
[ "Usage: too few args (delete)",   	    "xlat",  "-d",                                                                        "/dev/null", 1,  0 ],
[ "Usage: too many args (delete)",  	    "xlat",  "-d abcde 12345",                                                            "/dev/null", 1,  0 ],
[ "Usage: Complement not delete",   	    "xlat",  "-c abcde abcde",                                                            "/dev/null", 1,  0 ],
    # an easy one
[ "Translate a short file",         	    "xlat",  "aeiou AEIOU",                                                    		  "$inputdir/shortfile",    0,  0 ],    
[ "Delete vowels (short file)",     	    "xlat",  "-d aeiouAEIOU",                                                  		  "$inputdir/shortfile",    0,  0 ],    
[ "Delete nonvowels (short file)",  	    "xlat",  "-d -c aeiouAEIOU",                                               		  "$inputdir/shortfile",    0,  0 ],    
[ "Translate a long file",         	    "xlat",  "aeiou AEIOU",                                                    		  "$inputdir/longfile",     0,  1 ],    
[ "Delete vowels (long file)",     	    "xlat",  "-d aeiouAEIOU",                                                  		  "$inputdir/longfile",     0,  1 ],    
[ "Delete nonvowels (long file)",  	    "xlat",  "-d -c aeiouAEIOU",                                               		  "$inputdir/longfile",     0,  1 ],    
[ "Translate a all chars file",             "xlat",  "aeiou AEIOU",                                                    	  	  "$inputdir/allchars",     0,  0 ],    
[ "Delete vowels (all chars file)",         "xlat",  "-d aeiouAEIOU",                                                  	  	  "$inputdir/allchars",     0,  0 ],    
[ "Delete nonvowels (all chars file)",      "xlat",  "-d -c aeiouAEIOU",                                               	  	  "$inputdir/allchars",     0,  0 ],    
[ "Translate a long line",         	    "xlat",  "aeiou AEIOU",                                                    		  "$inputdir/verylongline", 0,  1 ],    
[ "Delete vowels (long line)",     	    "xlat",  "-d aeiouAEIOU",                                                  		  "$inputdir/verylongline", 0,  1 ],    
[ "Delete nonvowels (long line)",  	    "xlat",  "-d -c aeiouAEIOU",                                               		  "$inputdir/verylongline", 0,  1 ],
    # translate long set2
[ "Translate a short (long set2)",    	    "xlat",  "aeiou AEIOUextra",                                                    	  "$inputdir/shortfile",    0,  0 ],    
[ "Translate a long (long set2)",     	    "xlat",  "aeiou AEIOUextra",                                                    	  "$inputdir/longfile",     0,  1 ],    
[ "Translate a all chars (long set2)",	    "xlat",  "aeiou AEIOUextra",                                                    	  "$inputdir/allchars",     0,  0 ],    
[ "Translate a long line (long set2)",	    "xlat",  "aeiou AEIOUextra",                                                    	  "$inputdir/verylongline", 0,  1 ],    
# translate short set2
[ "Translate a short (long set2)", 	    "xlat",  "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ A",                    "$inputdir/shortfile",    0,  0 ],    
[ "Translate a long (long set2)",  	    "xlat",  "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ A",                    "$inputdir/longfile",     0,  1 ],    
[ "Translate a all chars (long set2)",	    "xlat",  "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ A",                    "$inputdir/allchars",     0,  0 ],    
[ "Translate a long line (long set2)",	    "xlat",  "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ A",                    "$inputdir/verylongline", 0,  1 ],    
# translate empty set1 (how?)
[ "Translate a short (empty set1)",    	    "xlat",  "'' aeiou",                                                    	  	  "$inputdir/shortfile",    0,  0 ],    
[ "Translate a long (empty set1)",     	    "xlat",  "'' aeiou",                                                    	  	  "$inputdir/longfile",     0,  1 ],    
[ "Translate a all chars (empty set1)",	    "xlat",  "'' aeiou",                                                  	  	  "$inputdir/allchars",     0,  0 ],    
[ "Translate a long line (empty set1)",	    "xlat",  "'' aeiou",                                                  	  	  "$inputdir/verylongline", 0,  1 ],    
# translate empty set2 (how?)
[ "Translate a short (empty set2)", 	    "xlat",  "aeiou ''",                                                    	  	  "$inputdir/shortfile",    1,  0 ],    
[ "Translate a long (empty set2)",  	    "xlat",  "aeiou ''",                                                    	  	  "$inputdir/longfile",     1,  1 ],    
[ "Translate a all chars (empty set2)",	    "xlat",  "aeiou ''",                                                  	  	  "$inputdir/allchars",     1,  0 ],    
[ "Translate a long line (empty set2)",	    "xlat",  "aeiou ''",                                                  	  	  "$inputdir/verylongline", 1,  1 ],    
# repeats in set1
[ "Translate a short (repeats set1)", 	    "xlat",  "abcabc aeioux",                                                    	  "$inputdir/shortfile",    0,  0 ],    
[ "Translate a long (repeats set1)",  	    "xlat",  "abcabc aeioux",                                                    	  "$inputdir/longfile",     0,  1 ],    
[ "Translate a all chars (repeats set1)",   "xlat",  "abcabc aeioux",                                                  	  	  "$inputdir/allchars",     0,  0 ],    
[ "Translate a long line (repeats set1)",   "xlat",  "abcabc aeioux",                                                  	  	  "$inputdir/verylongline", 0,  1 ],    
# repeats in set2
[ "Translate a short (repeats)",    	    "xlat",     "abcabc aaaaaa",                                                    	  "$inputdir/shortfile",    0,  0 ],    
[ "Translate a long (repeats)",     	    "xlat",     "abcabc aaaaaa",                                                    	  "$inputdir/longfile",     0,  1 ],    
[ "Translate a all chars (repeats)",	    "xlat",  "abcabc aaaaaa",                                                  	  	  "$inputdir/allchars",     0,  0 ],    
[ "Translate a long line (repeats)",	    "xlat",  "abcabc aaaaaa",                                                  	  	  "$inputdir/verylongline", 0,  1 ],    
# newlines in set one (catch line counters)
[ "newlines in set1 (short file)",          "xlat",  "'abc\ndef' xxxxxxx",                                      		  "$inputdir/shortfile",    0,  0 ],    
[ "newlines in set1 (long file)",           "xlat",  "'abc\ndef' xxxxxxx",                                                 	  "$inputdir/longfile",     0,  1 ],    
[ "newlines in set1 (all chars file)",      "xlat",  "'abc\ndef' xxxxxxx", 						   	  "$inputdir/allchars",     0,  0 ],    
[ "newlines in set1 (long line)",           "xlat",  "'abc\ndef' xxxxxxx",                                                 	  "$inputdir/verylongline", 0,  1 ],    
[ "newlines in set1 (very long file)",      "xlat",  "'abc\ndef' xxxxxxx",                                                 	  "$inputdir/verylongfile", 0,  1 ],    
# newlines in set two (catch line counters)
[ "newlines in set2 (short file)",          "xlat",  "xxxxxxx 'abc\ndef'",                                                 	  "$inputdir/shortfile",    0,  0 ],    
[ "newlines in set2 (long file)",           "xlat",  "xxxxxxx 'abc\ndef'",                                                 	  "$inputdir/longfile",     0,  1 ],    
[ "newlines in set2 (all chars file)",      "xlat",  "xxxxxxx 'abc\ndef'", 						   	  "$inputdir/allchars",     0,  0 ],    
[ "newlines in set2 (long line)",           "xlat",  "xxxxxxx 'abc\ndef'",                                                 	  "$inputdir/verylongline", 0,  1 ],    
[ "newlines in set2 (very long file)",      "xlat",  "xxxxxxx 'abc\ndef'",                                                 	  "$inputdir/verylongfile", 0,  1 ],    
# non-printable and negative characters
[ "wide trans alphabet (short file)",       "xlat",  "'$notquotes' '$notquotesbackwards'",                              	  "$inputdir/shortfile",    0,  0 ],    
[ "wide trans alphabet (long file)",        "xlat",  "'$notquotes' '$notquotesbackwards'",                              	  "$inputdir/longfile",     0,  1 ],    
[ "wide trans alphabet (all chars file)",   "xlat",  "'$notquotes' '$notquotesbackwards'", 					  "$inputdir/allchars",    0,  0 ],    
[ "wide trans alphabet (long line)",        "xlat",  "'$notquotes' '$notquotesbackwards'",                              	  "$inputdir/verylongline", 0,  1 ],    
[ "wide trans alphabet (very long file)",   "xlat",  "'$notquotes' '$notquotesbackwards'",                              	  "$inputdir/verylongfile", 0,  1 ],    
# A few more with a very long file
[ "Translate a very long file",             "xlat",  "aeiou AEIOU",                                                    		  "$inputdir/verylongfile", 0,  1 ],    
[ "Delete vowels (very long file)",         "xlat",  "-d aeiouAEIOU",                                                  		  "$inputdir/verylongfile", 0,  1 ],    
[ "Delete nonvowels (very long file)",      "xlat",  "-d -c aeiouAEIOU",                                               		  "$inputdir/verylongfile", 0,  1 ],    
[ "Translate very long file (long set2)",   "xlat",  "aeiou AEIOUextra",                                                    	  "$inputdir/verylongfile", 0,  1 ],    
[ "Translate very long file (long set1)",   "xlat",  "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ A",                    "$inputdir/verylongfile", 0,  1 ],    
[ "Translate very long file (empty set1)",  "xlat",  "'' aeiou",                                                    	  	  "$inputdir/verylongfile", 0,  1 ],    
[ "Translate very long file (empty set2)",  "xlat",  "aeiou ''",                                                    	  	  "$inputdir/verylongfile", 1,  1 ],    
[ "Translate very long file (repeats set1)","xlat",  "abcabc aeioux",                                                    	  "$inputdir/verylongfile", 0,  1 ],    
[ "Translate very long file (repeats)",     "xlat",  "abcabc aaaaaa",                                                    	  "$inputdir/verylongfile", 0,  1 ],    
# Files that are (or become) empty
[ "Translate an empty file",                "xlat",  "aeiou AEIOU",                                                    		  "$inputdir/empty", 	    0,  0 ],    
[ "Delete vowels from an empty file",       "xlat",  "-d aeiouAEIOU",                                                  		  "$inputdir/empty", 	    0,  0 ],    
[ "Delete all from a short file", 	    "xlat",  "-d -c ''",                                                    	  	  "$inputdir/shortfile",    0,  0 ],    
[ "Delete all from a long file",  	    "xlat",  "-d -c ''",                                                    	  	  "$inputdir/longfile",     0,  1 ],    
[ "Delete all from an all chars file",	    "xlat",  "-d -c ''",                                                  	  	  "$inputdir/allchars",     0,  0 ],    
[ "Delete all from a long line file",	    "xlat",  "-d -c ''",                                                  	  	  "$inputdir/verylongline", 0,  1 ],    
    
#	  [ "", 	"", "" ],
#	  [ "", 	"", "" ],
#	  [ "", 	"", "" ],
);


$logfile   = "logxlat.$$";
$difffile  = "diffxlat.$$";
$testfile  = "testxlat.$$";
$errfile  = "errxlat.$$";
$errors    = 0;
#$format = "   %-28s %-6s %-40s %4s\n";
#$format = "   %s\n      %-28s %-6s %-40s\n";
$format = "%-59s %8s\n       Cmd: %s %-40s \n";

$line="=======================================================================";
printf("   $line\n  ");
printf($format,"Test","Status","Prog","Arguments");
printf("   $line\n");
$errors += dotests(@tests);
printf("   $line\n");

$total=scalar(@tests);
$passed=$total-$errors;

if ( $errors == 0 ) {
    printf("Success. (%d/%d passed)\n",$passed,$total);
} else {
    printf("Failure. (%d/%d passed)\n",$passed,$total);
}

if ( $errors ) {
    printf("\n");
    printf("Actual ouput from tests and detailed results are below.\n");
    printf("\n");
    system("cat $logfile");
    unlink($logfile);
}

exit $errors;


sub dotests {
    local(@testarray) = @_;
    local($errors) = 0;
    $testnum=1;
    foreach $t ( @testarray ) {
	($name,$cmd,$args,$infile,$shouldfail,$quiet) = (@$t);
	if ( $nospaces ) {
	    # remove spaces between -p, -s, and the number
	    $_ = $args;
	    s/-p\s+(\d+)/-p$1/g;
	    s/-s\s+(\d+)/-s$1/g;
	    $args = $_;
	}
	unlink($testfile);
	unlink($errfile);
	unlink($difffile);
	if ( !$justonetest || ($testnum==$justonetest) ) {
	    $errors += onetest($testnum,$name,$cmd,$args,$infile,$shouldfail,$quiet);
	}
	$testnum++;
    }
    return $errors;
}

sub onetest {
    local($testnum,$name,$cmd,$args,$infile,$shouldfail,$quiet) = @_;
    local($oops)=0;

    if ( ! -x $submitted{$cmd} ) {
	# announce the test to the logfile
	open  (OUT,">> $logfile");
	print OUT "\n\n-----------------\n$testnum) $name:\n     $cmd " .
	    "$args \n-----------------\n";
	print OUT "Unable to locate $submitted{$cmd}\n";
	close (OUT);
	$failed = 1;
    } else {

#This is what it should be, but I'll have to regenerate the reference files
#because of scriptwrap
#
$command="captty -f $testfile -o $testfile -r $errfile -q -t $timeout -i $infile " . $submitted{$cmd} . " $args  ";

	$chatter = $debug;	# PLN
	if ( $chatter ) {
	    print "\n\n";
	    printf "CL: '%s'\n",pstring($command);
	    print "\n\n";
	}
	if ( $saveoutput ) {
	    open (CMD,">Output/cmd.$testnum");
	    printf CMD "TST: \"%s\"\n",pstring($command);
	    close(CMD);
	}

#	$status = system("$command");
	if ( 0 == ($pid = fork()) ) {
	    # Child
	    exec $command;
	    exit 1;		# nobody expects you to come back
	}
	wait();
	$status = $? >> 8;
    
	$failed = 0;

	if ( ! $status ) {	    # it exited zero.  A good sign
	    if ( $shouldfail ) { # unless it was supposed to fail...
		$failed = 1;
		open  (OUT,">> $logfile");
		printf OUT "\n\n-----------------\n$testnum) $name: \n     $cmd " .
		    "%s \n-----------------\n",pstring($args);
		print OUT " This test was expected to fail, but reports success.\n";
		print OUT " Perhaps a bad exit status or no error message?\n";
		close (OUT);
	    } else {
		# now see if it got the right output.
		$reffile = "$expdir/$testnum";
		if ( ! -f "$reffile" ) {
		    # create the reference file
		    $cl="captty -f $reffile -o $reffile -r $reffile -q -t $timeout -i $infile " . $reference{$cmd} . " $args ";
		    #print("\nCommand: \"$cl\"\n");
		    if ( 0 == ($pid = fork()) ) {
			# Child
			#print("PLN: refcl: $cl < $infile\n");
			exec $cl;
			exit 1;		# nobody expects you to come back
		    }
		    wait();
		    $status = $? >> 8;
		    if ( $saveoutput ) {
			open (CMD,">> Output/RefCmd.$testnum");
			print CMD "CMD: \"$cl\"\n";
			close(CMD);
		    }
		}
		$status=system("$diff $reffile $testfile > $difffile");
		
		if ( $status ) {# if different (and unexpected)
		    $failed = 1;		    
		    open  (OUT,">> $logfile");
		    printf OUT "\n\n-----------------\n$testnum) $name:\n     $cmd " .
			"%s \n-----------------\n",pstring($args);
		    open  (OUT,">> $logfile");
		    print OUT "Test program output:\n";
		    close (OUT);
		    if ( ! $quiet ) {
			system("cat -v $testfile >> $logfile");
		    } else {
			open  (OUT,">> $logfile");
			print OUT " --- Output limited to 10 lines. (There would be too much.) ---\n";
			close (OUT);
			system("cat -v $testfile | $iolimit >> $logfile 2>&1");
		    }
		    open  (OUT,">> $logfile");
		    print OUT "Diffs: ( < Reference > Submitted ):\n";
		    close (OUT);
		    if ( ! $quiet ) {
			system("cat -v $difffile >> $logfile");
		    } else {
			open  (OUT,">> $logfile");
			print OUT " --- Output limited to 10 lines. (There would be too much.) ---\n";
			close (OUT);
			system("cat -v $difffile | $iolimit >> $logfile 2>&1");
		    }
		}
	    }
	} else {
	    # PLN
	    #	open  (OUT,">> $logfile");
#		print OUT "\n\n-----------------\n$testnum) $name:\n     $cmd " .
#		    "$args \n-----------------\n";
#		print OUT "shouldfail is $shouldfail\n";
#		if ( -f $errfile ) {
#		    print OUT "$errfile doesn't exist\n";
#		} elsif ( -z $errfile ) {
#		    print OUT "-z $errfile is true\n";
#		} else {
#		    print OUT "-z $errfile is false\n";
#		}
#		close (OUT);
#		system("ls -l $errfile >> $logfile 2>&1");
	    # NLP

	    # program execution exited nonzero (failure)
	    if ( ! $shouldfail ) { 
		$failed = 1;
		open  (OUT,">> $logfile");
		printf OUT "\n\n-----------------\n$testnum) $name:\n     $cmd " .
		    "%s \n-----------------\n",pstring($args);
		print OUT "Program exited with bad status: $status\n\n";
		print OUT "Test program output:\n";
		close (OUT);
		if ( ! $quiet ) {
		    system("cat -v $testfile >> $logfile");
		} else {
		    open  (OUT,">> $logfile");
		    print OUT " --- Output limited to 10 lines. (There would be too much.) ---\n";
		    close (OUT);
		    system("cat -v $testfile | $iolimit >> $logfile 2>&1");
		}
		close (OUT);
	    } elsif ( -z $errfile ) {
		# there should be an error message if it was expected to fail and did
		$failed = 1;
		open  (OUT,">> $logfile");
		printf OUT "\n\n-----------------\n$testnum) $name:\n     $cmd " .
		    "%s \n-----------------\n",pstring($args);
		print OUT "Program was expected to fail, but produced no stderr output\n\n";
		print OUT "Test program output:\n";
		close (OUT);
		if ( ! $quiet ) {
		    system("cat -v $testfile >> $logfile");
		} else {
		    open  (OUT,">> $logfile");
		    print OUT " --- Output limited to 10 lines. (There would be too much.) ---\n";
		    close (OUT);
		    system("cat -v $testfile | $iolimit >> $logfile 2>&1");
		}
		close (OUT);
	    }
	}

        # report error output if any
        if ( $failed && -f $errfile && ! -z $errfile ) {
	    open  (OUT,">> $logfile");
	    print OUT "Submitted program's stderr:\n";
	    close(OUT);
	    if ( ! $quiet ) {
		system("cat -v $errfile >> $logfile");
	    } else {
		open  (OUT,">> $logfile");
		print OUT " --- Output limited to 10 lines. (There would be too much.) ---\n";
		close (OUT);
		system("cat -v $errfile | $iolimit >> $logfile 2>&1");
	    }
	    close (OUT);
	}


        if ( $saveoutput ) {
	    rename($testfile,"Output/test.$testnum.out");
	    rename($errfile,"Output/test.$testnum.err");
	} else {
	    unlink($testfile);
	    unlink($errfile);
	}
	unlink($difffile);

	if ( $chatter ) {
	    open  (OUT,">> $logfile");
	    print OUT "shouldfail is $shouldfail and failed is $failed\n";
	    if ( $shouldfail ) {
		$failed = $failed?0:1;
		print OUT "\tNow, it's $failed\n";
	    }
	    close(OUT);
	}
    }

    if ( $failed ) {
	$oops ++;
    }
    printf("%3d -- ",$testnum);
    $_ = $args;
    s/(\s)?$inputdir\//$1/;
    $args = $_;
    #	printf($format,$name,$cmd,$args,$status?"FAILED":"ok");
    if ( $infile =~ /(Inputs.*)/ ) {
	$in = " < $1";
    } else {
	$in = "";
    }
    printf($format,$name,$failed?"FAILED.":"ok.",$cmd,pstring($args) . $in);
#	printf($format,$status?"FAILED":"ok",$name,$cmd,$args);
#	last; 			# PLN
    return $oops;
}

sub usage {
    print STDERR "usage: txlat.pl [ -v ][ -n | --nospace ] [ -s | --save ] [ -f | --force ]\n";
    print STDERR "       -v --- increase verbosity\n";
    print STDERR "       -n --- don't put spaces between switches and arguments\n";
    print STDERR "       -s --- save test outputs for later comparison\n";
    print STDERR "       -f --- force recreation of the reference outputs (admin only)\n";
}

sub safechar {
    my($c) = @_;
    if ( $c =~ /([^[:print:]])/ ) {
	return sprintf("\\%03o",ord($1));
    } else {
	return "$c";
    }
}

sub pstring {
    my($s) = @_;
    my(@letters) = split //,$s;
    my($res,$c) = ("",0);
    foreach $c ( @letters ) {
	$res .= safechar($c);
    }
    return $res;
}
