<SETUP hborders=20>
<BKGCOLOR #004770>
<color 1>
<font small>
<customfromrecord _box_signinbox>
<record _box_signinbox><null><br>
<movecursor +0,+25>
<font normal><center>Create a new profile!<BR><movecursor +0,+15>
USERNAME:<sp 5><color #3f7493><TEXTBOX id=username default="" width=#width-250><null><BR><movecursor +0,+15>
<color 1>PASSWORD:<sp 5><color #3f7493><TEXTBOX id=password default="" width=#width-250><null><BR><movecursor +0,+15>
<color 1><sp 28>E-MAIL:<sp 5><color #3f7493><TEXTBOX id=email default="" width=#width-250><null><BR><color 1>
<br><movecursor +0,+10>
<link cmd=createprofile><custom 230,40 x:"CREATE PROFILE"></link><null><sp 15>
<link cmd="local:\\data\signin.ml"><custom 120,40 x:"CANCEL"></link><BR>
</record>
