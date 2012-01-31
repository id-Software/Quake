/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sbar.c -- status bar code

#include "quakedef.h"


int			sb_updates;		// if >= vid.numpages, no update needed

#define STAT_MINUS		10	// num frame for '-' stats digit
qpic_t		*sb_nums[2][11];
qpic_t		*sb_colon, *sb_slash;
qpic_t		*sb_ibar;
qpic_t		*sb_sbar;
qpic_t		*sb_scorebar;

qpic_t		*sb_weapons[7][8];	// 0 is active, 1 is owned, 2-5 are flashes
qpic_t		*sb_ammo[4];
qpic_t		*sb_sigil[4];
qpic_t		*sb_armor[3];
qpic_t		*sb_items[32];

qpic_t	*sb_faces[7][2];		// 0 is gibbed, 1 is dead, 2-6 are alive
							// 0 is static, 1 is temporary animation
qpic_t	*sb_face_invis;
qpic_t	*sb_face_quad;
qpic_t	*sb_face_invuln;
qpic_t	*sb_face_invis_invuln;

qboolean	sb_showscores;
qboolean	sb_showteamscores;

int			sb_lines;			// scan lines to draw

void Sbar_DeathmatchOverlay (int start);
void Sbar_TeamOverlay (void);
void Sbar_MiniDeathmatchOverlay (void);

static qboolean largegame = false;

/*
===============
Sbar_ShowTeamScores

Tab key down
===============
*/
void Sbar_ShowTeamScores (void)
{
	if (sb_showteamscores)
		return;

	sb_showteamscores = true;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowTeamScores

Tab key up
===============
*/
void Sbar_DontShowTeamScores (void)
{
	sb_showteamscores = false;
	sb_updates = 0;
}

/*
===============
Sbar_ShowScores

Tab key down
===============
*/
void Sbar_ShowScores (void)
{
	if (sb_showscores)
		return;

	sb_showscores = true;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowScores

Tab key up
===============
*/
void Sbar_DontShowScores (void)
{
	sb_showscores = false;
	sb_updates = 0;
}

/*
===============
Sbar_Changed
===============
*/
void Sbar_Changed (void)
{
	sb_updates = 0;	// update next frame
}

/*
===============
Sbar_Init
===============
*/
void Sbar_Init (void)
{
	int		i;

	for (i=0 ; i<10 ; i++)
	{
		sb_nums[0][i] = Draw_PicFromWad (va("num_%i",i));
		sb_nums[1][i] = Draw_PicFromWad (va("anum_%i",i));
	}

	sb_nums[0][10] = Draw_PicFromWad ("num_minus");
	sb_nums[1][10] = Draw_PicFromWad ("anum_minus");

	sb_colon = Draw_PicFromWad ("num_colon");
	sb_slash = Draw_PicFromWad ("num_slash");

	sb_weapons[0][0] = Draw_PicFromWad ("inv_shotgun");
	sb_weapons[0][1] = Draw_PicFromWad ("inv_sshotgun");
	sb_weapons[0][2] = Draw_PicFromWad ("inv_nailgun");
	sb_weapons[0][3] = Draw_PicFromWad ("inv_snailgun");
	sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
	sb_weapons[0][5] = Draw_PicFromWad ("inv_srlaunch");
	sb_weapons[0][6] = Draw_PicFromWad ("inv_lightng");
	
	sb_weapons[1][0] = Draw_PicFromWad ("inv2_shotgun");
	sb_weapons[1][1] = Draw_PicFromWad ("inv2_sshotgun");
	sb_weapons[1][2] = Draw_PicFromWad ("inv2_nailgun");
	sb_weapons[1][3] = Draw_PicFromWad ("inv2_snailgun");
	sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
	sb_weapons[1][5] = Draw_PicFromWad ("inv2_srlaunch");
	sb_weapons[1][6] = Draw_PicFromWad ("inv2_lightng");
	
	for (i=0 ; i<5 ; i++)
	{
		sb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_shotgun",i+1));
		sb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_sshotgun",i+1));
		sb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_nailgun",i+1));
		sb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_snailgun",i+1));
		sb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_rlaunch",i+1));
		sb_weapons[2+i][5] = Draw_PicFromWad (va("inva%i_srlaunch",i+1));
		sb_weapons[2+i][6] = Draw_PicFromWad (va("inva%i_lightng",i+1));
	}

	sb_ammo[0] = Draw_PicFromWad ("sb_shells");
	sb_ammo[1] = Draw_PicFromWad ("sb_nails");
	sb_ammo[2] = Draw_PicFromWad ("sb_rocket");
	sb_ammo[3] = Draw_PicFromWad ("sb_cells");

	sb_armor[0] = Draw_PicFromWad ("sb_armor1");
	sb_armor[1] = Draw_PicFromWad ("sb_armor2");
	sb_armor[2] = Draw_PicFromWad ("sb_armor3");

	sb_items[0] = Draw_PicFromWad ("sb_key1");
	sb_items[1] = Draw_PicFromWad ("sb_key2");
	sb_items[2] = Draw_PicFromWad ("sb_invis");
	sb_items[3] = Draw_PicFromWad ("sb_invuln");
	sb_items[4] = Draw_PicFromWad ("sb_suit");
	sb_items[5] = Draw_PicFromWad ("sb_quad");

	sb_sigil[0] = Draw_PicFromWad ("sb_sigil1");
	sb_sigil[1] = Draw_PicFromWad ("sb_sigil2");
	sb_sigil[2] = Draw_PicFromWad ("sb_sigil3");
	sb_sigil[3] = Draw_PicFromWad ("sb_sigil4");

	sb_faces[4][0] = Draw_PicFromWad ("face1");
	sb_faces[4][1] = Draw_PicFromWad ("face_p1");
	sb_faces[3][0] = Draw_PicFromWad ("face2");
	sb_faces[3][1] = Draw_PicFromWad ("face_p2");
	sb_faces[2][0] = Draw_PicFromWad ("face3");
	sb_faces[2][1] = Draw_PicFromWad ("face_p3");
	sb_faces[1][0] = Draw_PicFromWad ("face4");
	sb_faces[1][1] = Draw_PicFromWad ("face_p4");
	sb_faces[0][0] = Draw_PicFromWad ("face5");
	sb_faces[0][1] = Draw_PicFromWad ("face_p5");

	sb_face_invis = Draw_PicFromWad ("face_invis");
	sb_face_invuln = Draw_PicFromWad ("face_invul2");
	sb_face_invis_invuln = Draw_PicFromWad ("face_inv2");
	sb_face_quad = Draw_PicFromWad ("face_quad");

	Cmd_AddCommand ("+showscores", Sbar_ShowScores);
	Cmd_AddCommand ("-showscores", Sbar_DontShowScores);
		
	Cmd_AddCommand ("+showteamscores", Sbar_ShowTeamScores);
	Cmd_AddCommand ("-showteamscores", Sbar_DontShowTeamScores);
		
	sb_sbar = Draw_PicFromWad ("sbar");
	sb_ibar = Draw_PicFromWad ("ibar");
	sb_scorebar = Draw_PicFromWad ("scorebar");
}


//=============================================================================

// drawing routines are reletive to the status bar location

/*
=============
Sbar_DrawPic
=============
*/
void Sbar_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x /* + ((vid.width - 320)>>1) */, y + (vid.height-SBAR_HEIGHT), pic);
}

/*
=============
Sbar_DrawSubPic
=============
JACK: Draws a portion of the picture in the status bar.
*/

void Sbar_DrawSubPic(int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height) 
{
	Draw_SubPic (x, y+(vid.height-SBAR_HEIGHT), pic, srcx, srcy, width, height);
}


/*
=============
Sbar_DrawTransPic
=============
*/
void Sbar_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_TransPic (x /*+ ((vid.width - 320)>>1) */, y + (vid.height-SBAR_HEIGHT), pic);
}

/*
================
Sbar_DrawCharacter

Draws one solid graphics character
================
*/
void Sbar_DrawCharacter (int x, int y, int num)
{
	Draw_Character ( x /*+ ((vid.width - 320)>>1) */ + 4, y + vid.height-SBAR_HEIGHT, num);
}

/*
================
Sbar_DrawString
================
*/
void Sbar_DrawString (int x, int y, char *str)
{
	Draw_String (x /*+ ((vid.width - 320)>>1) */, y+ vid.height-SBAR_HEIGHT, str);
}

/*
=============
Sbar_itoa
=============
*/
int Sbar_itoa (int num, char *buf)
{
	char	*str;
	int		pow10;
	int		dig;
	
	str = buf;
	
	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}
	
	for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
	;
	
	do
	{
		pow10 /= 10;
		dig = num/pow10;
		*str++ = '0'+dig;
		num -= dig*pow10;
	} while (pow10 != 1);
	
	*str = 0;
	
	return str-buf;
}


/*
=============
Sbar_DrawNum
=============
*/
void Sbar_DrawNum (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Sbar_DrawTransPic (x,y,sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}

//=============================================================================

//ZOID: this should be MAX_CLIENTS, not MAX_SCOREBOARD!!
//int		fragsort[MAX_SCOREBOARD];
int		fragsort[MAX_CLIENTS];
int		scoreboardlines;
typedef struct {
	char team[16+1];
	int frags;
	int players;
	int plow, phigh, ptotal;
} team_t;
team_t teams[MAX_CLIENTS];
int teamsort[MAX_CLIENTS];
int scoreboardteams;

/*
===============
Sbar_SortFrags
===============
*/
void Sbar_SortFrags (qboolean includespec)
{
	int		i, j, k;
		
// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		if (cl.players[i].name[0] &&
			(!cl.players[i].spectator || includespec))
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
			if (cl.players[i].spectator)
				cl.players[i].frags = -999;
		}
	}
		
	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.players[fragsort[j]].frags < cl.players[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

void Sbar_SortTeams (void)
{
	int				i, j, k;
	player_info_t	*s;
	int				teamplay;
	char t[16+1];

// request new ping times every two second
	scoreboardteams = 0;

	teamplay = atoi(Info_ValueForKey(cl.serverinfo, "teamplay"));
	if (!teamplay)
		return;

// sort the teams
	memset(teams, 0, sizeof(teams));
	for (i = 0; i < MAX_CLIENTS; i++)
		teams[i].plow = 999;

	for (i = 0; i < MAX_CLIENTS; i++) {
		s = &cl.players[i];
		if (!s->name[0])
			continue;
		if (s->spectator)
			continue;

		// find his team in the list
		t[16] = 0;
		strncpy(t, Info_ValueForKey(s->userinfo, "team"), 16);
		if (!t || !t[0])
			continue; // not on team
		for (j = 0; j < scoreboardteams; j++)
			if (!strcmp(teams[j].team, t)) {
				teams[j].frags += s->frags;
				teams[j].players++;
				goto addpinginfo;
			}
		if (j == scoreboardteams) { // must add him
			j = scoreboardteams++;
			strcpy(teams[j].team, t);
			teams[j].frags = s->frags;
			teams[j].players = 1;
addpinginfo:
			if (teams[j].plow > s->ping)
				teams[j].plow = s->ping;
			if (teams[j].phigh < s->ping)
				teams[j].phigh = s->ping;
			teams[j].ptotal += s->ping;
		}
	}

	// sort
	for (i = 0; i < scoreboardteams; i++)
		teamsort[i] = i;

	// good 'ol bubble sort
	for (i = 0; i < scoreboardteams - 1; i++)
		for (j = i + 1; j < scoreboardteams; j++)
			if (teams[teamsort[i]].frags < teams[teamsort[j]].frags) {
				k = teamsort[i];
				teamsort[i] = teamsort[j];
				teamsort[j] = k;
			}
}

int	Sbar_ColorForMap (int m)
{
	m = (m < 0) ? 0 : ((m > 13) ? 13 : m);

	m *= 16;
	return m < 128 ? m + 8 : m + 8;
}


/*
===============
Sbar_SoloScoreboard
===============
*/
void Sbar_SoloScoreboard (void)
{
	char	str[80];
	int		minutes, seconds, tens, units;

	Sbar_DrawPic (0, 0, sb_scorebar);

	// time
	minutes = cl.time / 60;
	seconds = cl.time - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	sprintf (str,"Time :%3i:%i%i", minutes, tens, units);
	Sbar_DrawString (184, 4, str);
}

//=============================================================================

/*
===============
Sbar_DrawInventory
===============
*/
void Sbar_DrawInventory (void)
{	
	int		i;
	char	num[6];
	float	time;
	int		flashon;
	qboolean	headsup;
	qboolean    hudswap;

	headsup = !(cl_sbar.value || scr_viewsize.value<100);
	hudswap = cl_hudswap.value; // Get that nasty float out :)

	if (!headsup)
		Sbar_DrawPic (0, -24, sb_ibar);
// weapons
	for (i=0 ; i<7 ; i++)
	{
		if (cl.stats[STAT_ITEMS] & (IT_SHOTGUN<<i) )
		{
			time = cl.item_gettime[i];
			flashon = (int)((cl.time - time)*10);
			if (flashon < 0)
				flashon = 0;
			if (flashon >= 10)
			{
				if ( cl.stats[STAT_ACTIVEWEAPON] == (IT_SHOTGUN<<i)  )
					flashon = 1;
				else
					flashon = 0;
			}
			else
				flashon = (flashon%5) + 2;

			if (headsup) {
				if (i || vid.height>200)
					Sbar_DrawSubPic ((hudswap) ? 0 : (vid.width-24),-68-(7-i)*16 , sb_weapons[flashon][i],0,0,24,16);
			
			} else 
			Sbar_DrawPic (i*24, -16, sb_weapons[flashon][i]);
//			Sbar_DrawSubPic (0,0,20,20,i*24, -16, sb_weapons[flashon][i]);

			if (flashon > 1)
				sb_updates = 0;		// force update to remove flash
		}
	}

// ammo counts
	for (i=0 ; i<4 ; i++)
	{
		sprintf (num, "%3i",cl.stats[STAT_SHELLS+i] );
		if (headsup) {
//			Sbar_DrawSubPic(3, -24, sb_ibar, 3, 0, 42,11);
			Sbar_DrawSubPic((hudswap) ? 0 : (vid.width-42), -24 - (4-i)*11, sb_ibar, 3+(i*48), 0, 42, 11);
			if (num[0] != ' ')
				Sbar_DrawCharacter ( (hudswap) ? 3 : (vid.width-39), -24 - (4-i)*11, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter ( (hudswap) ? 11 : (vid.width-31), -24 - (4-i)*11, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter ( (hudswap) ? 19 : (vid.width-23), -24 - (4-i)*11, 18 + num[2] - '0');
		} else {
		if (num[0] != ' ')
			Sbar_DrawCharacter ( (6*i+1)*8 - 2, -24, 18 + num[0] - '0');
		if (num[1] != ' ')
			Sbar_DrawCharacter ( (6*i+2)*8 - 2, -24, 18 + num[1] - '0');
		if (num[2] != ' ')
			Sbar_DrawCharacter ( (6*i+3)*8 - 2, -24, 18 + num[2] - '0');
	}
	}
	
	flashon = 0;
// items
	for (i=0 ; i<6 ; i++)
		if (cl.stats[STAT_ITEMS] & (1<<(17+i)))
		{
			time = cl.item_gettime[17+i];
			if (time &&	time > cl.time - 2 && flashon )
			{	// flash frame
				sb_updates = 0;
			}
			else
				Sbar_DrawPic (192 + i*16, -16, sb_items[i]);		
			if (time &&	time > cl.time - 2)
				sb_updates = 0;
		}

// sigils
	for (i=0 ; i<4 ; i++)
		if (cl.stats[STAT_ITEMS] & (1<<(28+i)))
		{
			time = cl.item_gettime[28+i];
			if (time &&	time > cl.time - 2 && flashon )
			{	// flash frame
				sb_updates = 0;
			}
			else
				Sbar_DrawPic (320-32 + i*8, -16, sb_sigil[i]);		
			if (time &&	time > cl.time - 2)
				sb_updates = 0;
		}
}

//=============================================================================

/*
===============
Sbar_DrawFrags
===============
*/
void Sbar_DrawFrags (void)
{	
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	player_info_t	*s;
	
	Sbar_SortFrags (false);

// draw the text
	l = scoreboardlines <= 4 ? scoreboardlines : 4;
	
	x = 23;
//	xofs = (vid.width - 320)>>1;
	y = vid.height - SBAR_HEIGHT - 23;

	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.players[k];
		if (!s->name[0])
			continue;
		if (s->spectator)
			continue;

	// draw background
		top = s->topcolor;
		bottom = s->bottomcolor;
		top = (top < 0) ? 0 : ((top > 13) ? 13 : top);
		bottom = (bottom < 0) ? 0 : ((bottom > 13) ? 13 : bottom);

		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);
	
//		Draw_Fill (xofs + x*8 + 10, y, 28, 4, top);
//		Draw_Fill (xofs + x*8 + 10, y+4, 28, 3, bottom);
		Draw_Fill (x*8 + 10, y, 28, 4, top);
		Draw_Fill (x*8 + 10, y+4, 28, 3, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);
		
		Sbar_DrawCharacter ( (x+1)*8 , -24, num[0]);
		Sbar_DrawCharacter ( (x+2)*8 , -24, num[1]);
		Sbar_DrawCharacter ( (x+3)*8 , -24, num[2]);

		if (k == cl.playernum)
		{
			Sbar_DrawCharacter (x*8+2, -24, 16);
			Sbar_DrawCharacter ( (x+4)*8-4, -24, 17);
		}
		x+=4;
	}
}

//=============================================================================


/*
===============
Sbar_DrawFace
===============
*/
void Sbar_DrawFace (void)
{
	int		f, anim;

	if ( (cl.stats[STAT_ITEMS] & (IT_INVISIBILITY | IT_INVULNERABILITY) )
	== (IT_INVISIBILITY | IT_INVULNERABILITY) )
	{
		Sbar_DrawPic (112, 0, sb_face_invis_invuln);
		return;
	}
	if (cl.stats[STAT_ITEMS] & IT_QUAD) 
	{
		Sbar_DrawPic (112, 0, sb_face_quad );
		return;
	}
	if (cl.stats[STAT_ITEMS] & IT_INVISIBILITY) 
	{
		Sbar_DrawPic (112, 0, sb_face_invis );
		return;
	}
	if (cl.stats[STAT_ITEMS] & IT_INVULNERABILITY) 
	{
		Sbar_DrawPic (112, 0, sb_face_invuln);
		return;
	}

	if (cl.stats[STAT_HEALTH] >= 100)
		f = 4;
	else
		f = cl.stats[STAT_HEALTH] / 20;
	
	if (cl.time <= cl.faceanimtime)
	{
		anim = 1;
		sb_updates = 0;		// make sure the anim gets drawn over
	}
	else
		anim = 0;
	Sbar_DrawPic (112, 0, sb_faces[f][anim]);
}

/*
=============
Sbar_DrawNormal
=============
*/
void Sbar_DrawNormal (void)
{
	if (cl_sbar.value || scr_viewsize.value<100)
	Sbar_DrawPic (0, 0, sb_sbar);

// armor
	if (cl.stats[STAT_ITEMS] & IT_INVULNERABILITY)
	{
		Sbar_DrawNum (24, 0, 666, 3, 1);
		Sbar_DrawPic (0, 0, draw_disc);
	}
	else
	{
		Sbar_DrawNum (24, 0, cl.stats[STAT_ARMOR], 3
		, cl.stats[STAT_ARMOR] <= 25);
		if (cl.stats[STAT_ITEMS] & IT_ARMOR3)
			Sbar_DrawPic (0, 0, sb_armor[2]);
		else if (cl.stats[STAT_ITEMS] & IT_ARMOR2)
			Sbar_DrawPic (0, 0, sb_armor[1]);
		else if (cl.stats[STAT_ITEMS] & IT_ARMOR1)
			Sbar_DrawPic (0, 0, sb_armor[0]);
	}
	
// face
	Sbar_DrawFace ();
	
// health
	Sbar_DrawNum (136, 0, cl.stats[STAT_HEALTH], 3
	, cl.stats[STAT_HEALTH] <= 25);

// ammo icon
	if (cl.stats[STAT_ITEMS] & IT_SHELLS)
		Sbar_DrawPic (224, 0, sb_ammo[0]);
	else if (cl.stats[STAT_ITEMS] & IT_NAILS)
		Sbar_DrawPic (224, 0, sb_ammo[1]);
	else if (cl.stats[STAT_ITEMS] & IT_ROCKETS)
		Sbar_DrawPic (224, 0, sb_ammo[2]);
	else if (cl.stats[STAT_ITEMS] & IT_CELLS)
		Sbar_DrawPic (224, 0, sb_ammo[3]);
	
	Sbar_DrawNum (248, 0, cl.stats[STAT_AMMO], 3
	, cl.stats[STAT_AMMO] <= 10);
}

/*
===============
Sbar_Draw
===============
*/
void Sbar_Draw (void)
{
	qboolean headsup;
	char st[512];

	headsup = !(cl_sbar.value || scr_viewsize.value<100);
	if ((sb_updates >= vid.numpages) && !headsup)
		return;

	if (scr_con_current == vid.height)
		return;		// console is full screen

	scr_copyeverything = 1;
//	scr_fullupdate = 0;

	sb_updates++;
		
// top line
	if (sb_lines > 24)
	{
		if (!cl.spectator || autocam == CAM_TRACK)
			Sbar_DrawInventory ();
		if (!headsup || vid.width<512)
			Sbar_DrawFrags ();
	}	

// main area
	if (sb_lines > 0)
	{
		if (cl.spectator) {
			if (autocam != CAM_TRACK) {
				Sbar_DrawPic (0, 0, sb_scorebar);
				Sbar_DrawString (160-7*8,4, "SPECTATOR MODE");
				Sbar_DrawString(160-14*8+4, 12, "Press [ATTACK] for AutoCamera");
			} else {
				if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
					Sbar_SoloScoreboard ();
				else
					Sbar_DrawNormal ();

//					Sbar_DrawString (160-14*8+4,4, "SPECTATOR MODE - TRACK CAMERA");
				sprintf(st, "Tracking %-.13s, [JUMP] for next",
						cl.players[spec_track].name);
				Sbar_DrawString(0, -8, st);
			}
		} else if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
			Sbar_SoloScoreboard ();
		else
			Sbar_DrawNormal ();
	}

// main screen deathmatch rankings
	// if we're dead show team scores in team games
	if (cl.stats[STAT_HEALTH] <= 0 && !cl.spectator)
		if (atoi(Info_ValueForKey(cl.serverinfo, "teamplay")) > 0 &&
			!sb_showscores)
			Sbar_TeamOverlay();
		else
			Sbar_DeathmatchOverlay (0);
	else if (sb_showscores)
		Sbar_DeathmatchOverlay (0);
	else if (sb_showteamscores)
		Sbar_TeamOverlay();

#ifdef GLQUAKE
	if (sb_showscores || sb_showteamscores || 
		cl.stats[STAT_HEALTH] <= 0)
		sb_updates = 0;
	// clear unused areas in gl
#if 0
	{
		int x = (vid.width - 320)>>1;

		// left
		if (x > 0) {
			Draw_TileClear (0, vid.height - sb_lines, x, sb_lines);
			Draw_TileClear (x+320, vid.height - sb_lines, vid.width - x+320, sb_lines);
		}
	}
#endif
	if (vid.width > 320 && !headsup)
		Draw_TileClear (320, vid.height - sb_lines, vid.width - 320, sb_lines);
#endif

	if (sb_lines > 0)
		Sbar_MiniDeathmatchOverlay ();
}

//=============================================================================

/*
==================
Sbar_IntermissionNumber

==================
*/
void Sbar_IntermissionNumber (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Draw_TransPic (x,y,sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}

/*
==================
Sbar_TeamOverlay

team frags
added by Zoid
==================
*/
void Sbar_TeamOverlay (void)
{
	qpic_t			*pic;
	int				i, k, l;
	int				x, y;
	char			num[12];
	int				teamplay;
	char			team[5];
	team_t *tm;
	int plow, phigh, pavg;

// request new ping times every two second
	teamplay = atoi(Info_ValueForKey(cl.serverinfo, "teamplay"));

	if (!teamplay) {
		Sbar_DeathmatchOverlay(0);
		return;
	}

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	pic = Draw_CachePic ("gfx/ranking.lmp");
	Draw_Pic (160-pic->width/2, 0, pic);

	y = 24;
	x = 36;
	Draw_String(x, y, "low/avg/high team total players");
	y += 8;
//	Draw_String(x, y, "------------ ---- ----- -------");
	Draw_String(x, y, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f \x1d\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1e\x1e\x1f");
	y += 8;

// sort the teams
	Sbar_SortTeams();

// draw the text
	l = scoreboardlines;

	for (i=0 ; i < scoreboardteams && y <= vid.height-10 ; i++)
	{
		k = teamsort[i];
		tm = teams + k;

	// draw pings
		plow = tm->plow;
		if (plow < 0 || plow > 999)
			plow = 999;
		phigh = tm->phigh;
		if (phigh < 0 || phigh > 999)
			phigh = 999;
		if (!tm->players)
			pavg = 999;
		else
			pavg = tm->ptotal / tm->players;
		if (pavg < 0 || pavg > 999)
			pavg = 999;

		sprintf (num, "%3i/%3i/%3i", plow, pavg, phigh);
		Draw_String ( x, y, num);

	// draw team
		team[4] = 0;
		strncpy (team, tm->team, 4);
		Draw_String (x + 104, y, team);

	// draw total
		sprintf (num, "%5i", tm->frags);
		Draw_String (x + 104 + 40, y, num);
		
	// draw players
		sprintf (num, "%5i", tm->players);
		Draw_String (x + 104 + 88, y, num);
		
		if (!strncmp(Info_ValueForKey(cl.players[cl.playernum].userinfo,
			"team"), tm->team, 16)) {
			Draw_Character ( x + 104 - 8, y, 16);
			Draw_Character ( x + 104 + 32, y, 17);
		}
		
		y += 8;
	}
	y += 8;
	Sbar_DeathmatchOverlay(y);
}

/*
==================
Sbar_DeathmatchOverlay

ping time frags name
==================
*/
void Sbar_DeathmatchOverlay (int start)
{
	qpic_t			*pic;
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	player_info_t	*s;
	int				total;
	int				minutes;
	int				p;
	int				teamplay;
	char			team[5];
	int				skip = 10;

	if (largegame)
		skip = 8;

// request new ping times every two second
	if (realtime - cl.last_ping_request > 2)
	{
		cl.last_ping_request = realtime;
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, "pings");
	}

	teamplay = atoi(Info_ValueForKey(cl.serverinfo, "teamplay"));

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	if (!start) {
		pic = Draw_CachePic ("gfx/ranking.lmp");
		Draw_Pic (160-pic->width/2, 0, pic);
	}

// scores	
	Sbar_SortFrags (true);

// draw the text
	l = scoreboardlines;

	if (start)
		y = start;
	else
		y = 24;
	if (teamplay)
	{
		x = 4;
//                            0    40 64   104   152  192 
		Draw_String ( x , y, "ping pl time frags team name");
		y += 8;
//		Draw_String ( x , y, "---- -- ---- ----- ---- ----------------");
		Draw_String ( x , y, "\x1d\x1e\x1e\x1f \x1d\x1f \x1d\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1f \x1d\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f");
		y += 8;
	}
	else
	{
		x = 16;
//                            0    40 64   104   152
		Draw_String ( x , y, "ping pl time frags name");
		y += 8;
//		Draw_String ( x , y, "---- -- ---- ----- ----------------");
		Draw_String ( x , y, "\x1d\x1e\x1e\x1f \x1d\x1f \x1d\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f");
		y += 8;
	}

	for (i=0 ; i<l && y <= vid.height-10 ; i++)
	{
		k = fragsort[i];
		s = &cl.players[k];
		if (!s->name[0])
			continue;

		// draw ping
		p = s->ping;
		if (p < 0 || p > 999)
			p = 999;
		sprintf (num, "%4i", p);
		Draw_String ( x, y, num);

		// draw pl
		p = s->pl;
		sprintf (num, "%3i", p);
		if (p > 25)
			Draw_Alt_String ( x+32, y, num);
		else
			Draw_String ( x+32, y, num);

		if (s->spectator)
		{
			Draw_String (x+40, y, "(spectator)");
			// draw name
			if (teamplay)
				Draw_String (x+152+40, y, s->name);
			else
				Draw_String (x+152, y, s->name);
			y += skip;
			continue;
		}


		// draw time
		if (cl.intermission)
			total = cl.completed_time - s->entertime;
		else
			total = realtime - s->entertime;
		minutes = (int)total/60;
		sprintf (num, "%4i", minutes);
		Draw_String ( x+64 , y, num);

		// draw background
		top = s->topcolor;
		bottom = s->bottomcolor;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);
	
		if (largegame)
			Draw_Fill ( x+104, y+1, 40, 3, top);
		else
			Draw_Fill ( x+104, y, 40, 4, top);
		Draw_Fill ( x+104, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);
		
		Draw_Character ( x+112 , y, num[0]);
		Draw_Character ( x+120 , y, num[1]);
		Draw_Character ( x+128 , y, num[2]);

		if (k == cl.playernum)
		{
			Draw_Character ( x + 104, y, 16);
			Draw_Character ( x + 136, y, 17);
		}
		
		// team
		if (teamplay)
		{
			team[4] = 0;
			strncpy (team, Info_ValueForKey(s->userinfo, "team"), 4);
			Draw_String (x+152, y, team);
		}

		// draw name
		if (teamplay)
			Draw_String (x+152+40, y, s->name);
		else
			Draw_String (x+152, y, s->name);
		
		y += skip;
	}

	if (y >= vid.height-10) // we ran over the screen size, squish
		largegame = true;
}

/*
==================
Sbar_MiniDeathmatchOverlay

frags name
frags team name
displayed to right of status bar if there's room
==================
*/
void Sbar_MiniDeathmatchOverlay (void)
{
	int				i, k;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	player_info_t	*s;
	int				teamplay;
	char			team[5];
	int				numlines;
	char			name[16+1];
	team_t			*tm;

	if (vid.width < 512 || !sb_lines)
		return; // not enuff room

	teamplay = atoi(Info_ValueForKey(cl.serverinfo, "teamplay"));

	scr_copyeverything = 1;
	scr_fullupdate = 0;

// scores	
	Sbar_SortFrags (false);
	if (vid.width >= 640)
		Sbar_SortTeams();

	if (!scoreboardlines)
		return; // no one there?

// draw the text
	y = vid.height - sb_lines - 1;
	numlines = sb_lines/8;
	if (numlines < 3)
		return; // not enough room

	// find us
	for (i=0 ; i < scoreboardlines; i++)
		if (fragsort[i] == cl.playernum)
			break;

	if (i == scoreboardlines) // we're not there, we are probably a spectator, just display top
		i = 0;
	else // figure out start
		i = i - numlines/2;

	if (i > scoreboardlines - numlines)
		i = scoreboardlines - numlines;
	if (i < 0)
		i = 0;

	x = 324;

	for (/* */ ; i < scoreboardlines && y < vid.height - 8 + 1; i++)
	{
		k = fragsort[i];
		s = &cl.players[k];
		if (!s->name[0])
			continue;

	// draw ping
		top = s->topcolor;
		bottom = s->bottomcolor;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);
	
		Draw_Fill ( x, y+1, 40, 3, top);
		Draw_Fill ( x, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);
		
		Draw_Character ( x+8 , y, num[0]);
		Draw_Character ( x+16, y, num[1]);
		Draw_Character ( x+24, y, num[2]);

		if (k == cl.playernum)
		{
			Draw_Character ( x, y, 16);
			Draw_Character ( x + 32, y, 17);
		}
		
	// team
		if (teamplay)
		{
			team[4] = 0;
			strncpy (team, Info_ValueForKey(s->userinfo, "team"), 4);
			Draw_String (x+48, y, team);
		}

	// draw name
		name[16] = 0;
		strncpy(name, s->name, 16);
		if (teamplay)
			Draw_String (x+48+40, y, name);
		else
			Draw_String (x+48, y, name);
		y += 8;
	}

	// draw teams if room
	if (vid.width < 640 || !teamplay)
		return;

	// draw seperator
	x += 208;
	for (y = vid.height - sb_lines; y < vid.height - 6; y += 2)
		Draw_Character(x, y, 14);

	x += 16;

	y = vid.height - sb_lines;
	for (i=0 ; i < scoreboardteams && y <= vid.height; i++)
	{
		k = teamsort[i];
		tm = teams + k;

	// draw pings
		team[4] = 0;
		strncpy (team, tm->team, 4);
		Draw_String (x, y, team);

	// draw total
		sprintf (num, "%5i", tm->frags);
		Draw_String (x + 40, y, num);
		
		if (!strncmp(Info_ValueForKey(cl.players[cl.playernum].userinfo,
			"team"), tm->team, 16)) {
			Draw_Character ( x - 8, y, 16);
			Draw_Character ( x + 32, y, 17);
		}
		
		y += 8;
	}

}


/*
==================
Sbar_IntermissionOverlay

==================
*/
void Sbar_IntermissionOverlay (void)
{
	scr_copyeverything = 1;
	scr_fullupdate = 0;

	if (atoi(Info_ValueForKey(cl.serverinfo, "teamplay")) > 0 && !sb_showscores)
		Sbar_TeamOverlay ();
	else
		Sbar_DeathmatchOverlay (0);
}


/*
==================
Sbar_FinaleOverlay

==================
*/
void Sbar_FinaleOverlay (void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;

	pic = Draw_CachePic ("gfx/finale.lmp");
	Draw_TransPic ( (vid.width-pic->width)/2, 16, pic);
}


