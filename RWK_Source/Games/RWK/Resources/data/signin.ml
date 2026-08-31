<SETUP hborders=20>
<BKGCOLOR #004770>
<color 1>
<font small>
<customfromrecord _box_signinbox>
<record _box_signinbox><null><br><br><BR>
<movecursor +0,+10>
<font normal><center>
USERNAME:<sp 5><color #3f7493><TEXTBOX id=username default="#username" width=#width-250><null><BR><movecursor +0,+15>
<color 1>PASSWORD:<sp 5><color #3f7493><TEXTBOX id=password default="#password" width=#width-250><null><BR><color 1>
<br>
<confine 40% 90%>
<link cmd=signin><custom 230,40 x:"SIGN IN NOW"></link><BR>
<movecursor +0,+8>
<color 1,1,1,.5><font small><link cmd="local:\\data\recover.ml">Forgot name or password?</link><font tiny><br>
</confine>
</record>
<center>
<font small>
<BR><BR>
<color 1,1,1,.75>No profile? Create one, quick and easy!<color 1><BR>
<null><movecursor +0,+10>
<link cmd="local:\\data\createprofile.ml"><custom 230,40 x:"CREATE PROFILE"></link><null><BR><BR>

