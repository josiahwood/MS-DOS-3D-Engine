//3D Engine

//#include <conio.h>
#include <graphics.h>
#include <math.h>
#include <bios.h>
#include "a:\turbo\keycodes.h"
#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
//#include <dir.h>

//#define OPTSOUND
//#define PAGEFLIP
#define RECORDLOG
#define OPTRADAR

int logtab=0;
fstream f;

void clearlog()
{
#ifdef RECORDLOG
	f.open("c:\\tc\\log.txt",ios::out);
	f.close();
#endif
}

void log(char *text)
{
#ifdef RECORDLOG
	f.open("c:\\tc\\log.txt",ios::app);	//append
	for(int x=0;x<logtab-1;x++)
		f<<" ";
	f<<text<<endl;
	f.close();
#endif
}

const double wall=1;
const char worldfile[]="a:\\turbo\\world.txt";

//User Options
int opt_radartype=1;	//0=stationary
			//1=rotates
int opt_radarradius=200;
int opt_sound=0;	//sound

//Sound
int soundident=0;	//0=none
			//1=MGUN
			//2=LLSR
			//3=MLSR
			//4=SLSR

//Weapon Types
char wnn[4][5]={"MGUN","LLSR","MLSR","SLSR"};
const int wnt_MGUN=0;
const int wnt_LLSR=1;
const int wnt_MLSR=2;
const int wnt_SLSR=3;
const int wnh[4]={0,15,10,5};
const int wnd[4]={1,5,3,2};
int wna[4]={-1/*100*/,-1,-1,-1};//ammo
const int wnal[4]={0/*1*/,0,0,0};//ammo loss per repeat
const int wnar[4]={1/*5*/,1,1,1};//ammo repeat
const int wn_fd[4]={1/*2*/,1,1,1};//fire delay between repetitions
int wn_fdc=0;//fire delay counter

//Weapon Info
int wnt[4]={wnt_LLSR,wnt_LLSR,wnt_MLSR,wnt_SLSR};	//weapon type array
int wn_cur=0;
int wn_rpt=0;

//Speed
double sper=0;

//Walking
double wdist=0;		//distance walked
double sdist=10; 	//step distance
double sheight=1;	//step height
double mh=1;		//mech height

int MechIndex=0;	//group index of player mech

//Heat
double m_heat=0;

//Function Headers
void OptionsMenu();
void DrawOptions(int sel);
void MoveMech();
void MoveObjects();
double sqr(double);

int matherr (struct exception *a)
{
  if (a->type == DOMAIN)
    if (!strcmp(a->name,"sqrt")) {
      a->retval = sqrt (-(a->arg1));
    return 1;
    }
  return 0;
}

void ERR_MEM()
{
#ifdef OPTSOUND
	nosound();
#endif

	cout<<"Not enough memory"<<endl;
	//getch();
	exit(0);
}

class CPolygon
{
public:
	int color;
	int fillstyle;
	int PointCount;
	double* x;
	double* y;
	double* z;

	double dist;	//for layering purposes

	CPolygon();
	virtual ~CPolygon();

	CPolygon& operator =(const CPolygon& aPolygon);
};

CPolygon::CPolygon()
{
	PointCount=0;
	color=0;
	fillstyle=1;	//solid
	dist=0;
}

CPolygon::~CPolygon()
{
	if(PointCount>0)
	{
		delete[]x;
		delete[]y;
		delete[]z;
		PointCount=0;
	}
}

CPolygon& CPolygon::operator =(const CPolygon& aPolygon)
{
	if(PointCount>0)
	{
		delete[]x;
		delete[]y;
		delete[]z;
		PointCount=0;
	}
	PointCount=aPolygon.PointCount;
	if(!(x=new double[PointCount]))
		ERR_MEM();
	if(!(y=new double[PointCount]))
		ERR_MEM();
	if(!(z=new double[PointCount]))
		ERR_MEM();
	//if(PointCount<100)
	for(int a=0;a<PointCount;a++)
	{
		x[a]=aPolygon.x[a];
		y[a]=aPolygon.y[a];
		z[a]=aPolygon.z[a];
	}
	//else
	//	PointCount=0;

	color=aPolygon.color;
	fillstyle=aPolygon.fillstyle;
	dist=aPolygon.dist;

	return *this;
}

class CGroup
{
public:
	int PolygonCount;
	CPolygon* Polygons;
	double a,t,p;
	int Visible;
	int radar; 	//0=none
			//1=friend
			//2=foe
			//3=neutral
			//4=weapon
	int ident;	//0=Player Mech
			//1=Friend Mech
			//2=Foe Mech
			//3=Neutral Object
			//4=Weapon
			//5=walls
	//weapon
	int weapon;	//0=MGUN
			//1=LLSR
			//2=MLSR
			//3=SLSR

	int TagCount;
	int* Tags;

	double x,y,z;

	CGroup();
	virtual ~CGroup();

	CGroup& operator =(const CGroup& aGroup);
};

CGroup::CGroup()
{
	x=0;
	y=0;
	z=0;
	a=0;
	t=0;
	p=0;
	PolygonCount=0;
	TagCount=0;
	weapon=-1;
	Visible=1;
	radar=0;
	ident=0;
}

CGroup::~CGroup()
{
	if(PolygonCount>0)
	{
		delete[]Polygons;
		PolygonCount=0;
	}
	if(TagCount>0)
	{
		delete[]Tags;
		TagCount=0;
	}
}

CGroup& CGroup::operator =(const CGroup& aGroup)
{
	if(PolygonCount>0)
	{
		delete[]Polygons;
		PolygonCount=0;
	}
	if(TagCount>0)
	{
		delete[]Tags;
		TagCount=0;
	}
	a=aGroup.a;
	p=aGroup.p;
	PolygonCount=aGroup.PolygonCount;
	TagCount=aGroup.TagCount;
	if(!(Polygons=new CPolygon[PolygonCount]))
		ERR_MEM();
	for(int c=0;c<PolygonCount;c++)
		Polygons[c]=aGroup.Polygons[c];

	if(!(Tags=new int[TagCount]))
		ERR_MEM();
	for(c=0;c<TagCount;c++)
		Tags[c]=aGroup.Tags[c];

	t=aGroup.t;
	Visible=aGroup.Visible;
	x=aGroup.x;
	y=aGroup.y;
	z=aGroup.z;
	radar=aGroup.radar;
	ident=aGroup.ident;
	weapon=aGroup.weapon;

	return *this;
}

class CWorld
{
public:
	CGroup* Groups;
	int GroupCount;

	CGroup* GTemps;
	int GTempCount;

	//Camera Position
	double cx,cy,cz;

	//Camera Direction
	double ca,ct;

	//Camera Zoom
	double cw,ch;

	//View Dimensions
	double vw,vh;

	int page;

	void Draw();
	void DrawRadar();
	void DrawCursor();
	void DrawWeapons();
	void DrawSpeed();
	void DrawHeat();
	void DrawPolygons();
	void DrawPolygon(CPolygon* Polygon);
	void AddPolygon(CPolygon* Polygon);
	void AddGroup(CGroup* Group);
	void DeleteGroup(int index);

	void Fire();

	int PolygonCount;
	int PolygonsSize;
	CPolygon* Polygons;

	double GetA(double x,double y);
	void LoadGroups(char* file,int length);

	CWorld();
	virtual ~CWorld();
}world;

CWorld::~CWorld()
{
	if(GroupCount>0)
		delete[]Groups;
	if(PolygonsSize>0)
		delete[]Polygons;
	if(GTempCount>0)
		delete[]GTemps;
}

CWorld::CWorld()
{
	PolygonCount=0;
	PolygonsSize=0;
	GroupCount=0;
	GTempCount=0;
	cx=0;
	cy=0;
	cz=0;
	ca=90;
	ct=0;
}

void CWorld::Draw()
{
	logtab++;
	log("start Draw");
	logtab++;

#ifdef PAGEFLIP
	page=!page;
	setactivepage(page);
#endif

	if(ca<0)
		ca+=360;
	else if(ca>=360)
		ca-=360;
	if(ct<0)
		ct+=360;
	else if(ct>=360)
		ct-=360;

	PolygonCount=0;

	CPolygon pg;
	double a,t,p,d;

	int x,y,z;

	for(x=0;x<GroupCount;x++)
	{
		if(Groups[x].Visible)
		{
			for(y=0;y<Groups[x].PolygonCount;y++)
			{
				logtab++;
				pg=Groups[x].Polygons[y];

				for(z=0;z<Groups[x].Polygons[y].PointCount;z++)
				{
					if(pg.y[z]!=0 || pg.z[z]!=0)
					{
						d=hypot(pg.y[z],pg.z[z]);
						t=GetA(pg.y[z],pg.z[z]);
						t+=Groups[x].t;

						pg.y[z]=cos(t/180*M_PI)*d;
						pg.z[z]=sin(t/180*M_PI)*d;
					}
					if(pg.x[z]!=0 || pg.z[z]!=0)
					{
						d=hypot(pg.x[z],pg.z[z]);
						p=GetA(pg.x[z],pg.z[z]);
						p+=Groups[x].p;

						pg.x[z]=cos(p/180*M_PI)*d;
						pg.z[z]=sin(p/180*M_PI)*d;
					}
					if(pg.x[z]!=0 || pg.y[z]!=0)
					{
						d=hypot(pg.x[z],pg.y[z]);
						a=GetA(pg.x[z],pg.y[z]);
						a+=Groups[x].a;

						pg.x[z]=cos(a/180*M_PI)*d;
						pg.y[z]=sin(a/180*M_PI)*d;
					}
					pg.x[z]+=Groups[x].x;
					pg.y[z]+=Groups[x].y;
					pg.z[z]+=Groups[x].z;
				}
				log("flag  done transforming polygon");
				AddPolygon(&pg);
				logtab--;
			}
		}
	}
	log("flag  done transforming polygons");
	DrawPolygons();
	DrawRadar();
	DrawCursor();
	DrawWeapons();
	DrawSpeed();
	DrawHeat();
	setcolor(15);
	rectangle(0,0,vw,vh);

#ifdef PAGEFLIP
	setvisualpage(page);
#endif
	logtab--;
	log("end   Draw");
	logtab--;
}

void CWorld::DrawRadar()
{
#ifdef OPTRADAR
	logtab++;
	log("start DrawRadar");

	double d,a;

	setfillstyle(1,0);
	setcolor(15);
	fillellipse(52,123,37,37);
	setfillstyle(1,2);
	bar(51,122,53,124);

	double x=0,y=0,color=0,f,g;

	double l1x,l1y,l2x,l2y,a1,a2;
	if(opt_radartype)
		a1=90-(cw/2);
	else
		a1=ca-(cw/2);

	if(a1<0)
		a1+=360;
	else if(a1>=360)
		a1-=360;
	l1x=cos(a1/180*M_PI)*37+52;
	l1y=123-sin(a1/180*M_PI)*37;

	if(opt_radartype)
		a2=90+(cw/2);
	else
		a2=ca+(cw/2);

	if(a2<0)
		a2+=360;
	else if(a2>=360)
		a2-=360;
	l2x=cos(a2/180*M_PI)*37+52;
	l2y=123-sin(a2/180*M_PI)*37;

	line(52,123,l1x,l1y);
	line(52,123,l2x,l2y);

	for(int c=0;c<GroupCount;c++)
	{
		if(Groups[c].Visible)
		{
			switch(Groups[c].radar)
			{
			case 0://none
				color=0;
				break;
			case 1://friend
				color=2;
				break;
			case 2://foe
				color=4;
				break;
			case 3://neutral
				color=14;
				break;
			case 4://weapon
				color=15;
				break;
			case 5:	//walls
				color=15;
				break;
			default:
				color=15;
			}
			setcolor(color);

			for(int e=0;e<Groups[c].PolygonCount;e++)
			{
				double lastx=Groups[c].Polygons[e].x[Groups[c].Polygons[e].PointCount-1];
				double lasty=Groups[c].Polygons[e].y[Groups[c].Polygons[e].PointCount-1];

				for(int d=0;d<Groups[c].Polygons[e].PointCount;d++)
				{
					double x1=lastx;
					double y1=lasty;
					double x2=Groups[c].Polygons[e].x[d];
					double y2=Groups[c].Polygons[e].y[d];

					double q,r;

					if(x1!=0 || y1!=0)
					{
						r=hypot(x1,y1);
						q=GetA(x1,y1);
						q+=Groups[c].a;

						x1=cos(q/180*M_PI)*r;
						y1=sin(q/180*M_PI)*r;
					}

					if(x2!=0 || y2!=0)
					{
						r=hypot(x2,y2);
						q=GetA(x2,y2);
						q+=Groups[c].a;

						x2=cos(q/180*M_PI)*r;
						y2=sin(q/180*M_PI)*r;
					}

					x1-=cx;
					y1-=cy;
					x2-=cx;
					y2-=cy;
					x1+=Groups[c].x;
					y1+=Groups[c].y;
					x2+=Groups[c].x;
					y2+=Groups[c].y;

					if(hypot(x1,y1)>hypot(x2,y2))
						g=hypot(x2,y2);
					else
						g=hypot(x1,y1);
					x1=x1/opt_radarradius*37.0;
					y1=y1/opt_radarradius*37.0;
					x2=x2/opt_radarradius*37.0;
					y2=y2/opt_radarradius*37.0;

					if(opt_radartype)
					{
						f=hypot(x1,y1);

						a=GetA(x1,y1);
						a=90+(a-ca);

						if(a<0)
							a+=360;
						else if(a>=360)
							a-=360;

						x1=cos(a/180*M_PI)*f;
						y1=sin(a/180*M_PI)*f;

						f=hypot(x2,y2);

						a=GetA(x2,y2);
						a=90+(a-ca);

						if(a<0)
							a+=360;
						else if(a>=360)
							a-=360;

						x2=cos(a/180*M_PI)*f;
						y2=sin(a/180*M_PI)*f;
					}

					if(g<opt_radarradius)
						line(x1+52,123-y1,x2+52,123-y2);
					lastx=Groups[c].Polygons[e].x[d];
					lasty=Groups[c].Polygons[e].y[d];
				}
			}
		}
		/*else
		{
			x=Groups[c].x-cx;
			y=Groups[c].y-cy;

			if(opt_radartype)
			{
				d=hypot(x,y);

				a=GetA(x,y);
				a=90+(a-ca);

				if(a<0)
					a+=360;
				else if(a>=360)
					a-=360;

				x=cos(a/180*M_PI)*d;
				y=sin(a/180*M_PI)*d;
			}

			switch(Groups[c].radar)
			{
			case 0://none
				color=0;
				break;
			case 1://friend
				color=2;
				break;
			case 2://foe
				color=4;
				break;
			case 3://neutral
				color=14;
				break;
			case 4://weapon
				color=15;
				break;
			default:
				color=15;
			}

			if((hypot(x,y)<opt_radarradius) && Groups[c].Visible)
			{
				x=x/opt_radarradius*37.0;
				y=y/opt_radarradius*37.0;
				setfillstyle(1,color);
				bar(x+51,122-y,x+53,124-y);
			}
		}*/
	}

	log("end   DrawRadar");
	logtab--;
#endif
}

void CWorld::DrawCursor()
{
	logtab++;
	log("start DrawCursor");

	int curx,cury;
	curx=vw/2;

	cury=vh/2;
	setcolor(7);
	circle(curx,cury,5);
	line(curx-7,cury,curx-2,cury);
	line(curx+7,cury,curx+2,cury);
	line(curx,cury-2,curx,cury-7);
	line(curx,cury+2,curx,cury+7);

	if(ct<90)
		cury=vh-(ct/ch*vh+(vh/2));
	else
		cury=vh-((ct-360)/ch*vh+(vh/2));

	setcolor(15);
	circle(curx,cury,5);
	line(curx-7,cury,curx-2,cury);
	line(curx+7,cury,curx+2,cury);
	line(curx,cury-2,curx,cury-7);
	line(curx,cury+2,curx,cury+7);

	log("end   DrawCursor");
	logtab--;
}

void CWorld::DrawWeapons()
{
	char b[5];
	if(wn_cur==0)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(vw-150,5,wnn[wnt[0]]);
	if(wna[0]!=-1)
		outtextxy(vw-110,5,itoa(wna[0],b,10));
	if(wn_cur==1)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(vw-75,5,wnn[wnt[1]]);
	if(wna[1]!=-1)
		outtextxy(vw-35,5,itoa(wna[0],b,10));
	if(wn_cur==2)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(vw-150,15,wnn[wnt[2]]);
	if(wna[2]!=-1)
		outtextxy(vw-110,15,itoa(wna[0],b,10));
	if(wn_cur==3)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(vw-75,15,wnn[wnt[3]]);
	if(wna[3]!=-1)
		outtextxy(vw-35,15,itoa(wna[0],b,10));
}

void CWorld::DrawHeat()
{
	setcolor(14);
	setfillstyle(1,14);	//yellow
	pieslice(vw-100,vh-50,0,(m_heat+wnh[wnt[wn_cur]])/100.0*360.0,20);
	setcolor(4);
	setfillstyle(1,4);	//red
	pieslice(vw-100,vh-50,0,m_heat/100.0*360.0,20);
	setfillstyle(1,0);
	fillellipse(vw-100,vh-50,10,10);
	char b[20];
	outtextxy(5,5,gcvt(ct,10,b));
	outtextxy(5,15,gcvt(ca,10,b));
}

void CWorld::DrawSpeed()
{
	setcolor(4);
	setfillstyle(1,4);	//red
	if(sper>=0)
		pieslice(vw-50,vh-50,0,sper*360.0,20);
	else
		pieslice(vw-50,vh-50,(1+sper)*360.0,360,20);
	setfillstyle(1,0);
	fillellipse(vw-50,vh-50,10,10);
}

void CWorld::Fire()
{
	int x=0;
	if(wna[wn_cur]>=wnal[wn_cur])
		wna[wn_cur]-=wnal[wn_cur];
	if(wn_rpt==0)
		wn_rpt=wnar[wn_cur];

	m_heat+=wnh[wnt[wn_cur]];
	int found=0;

	for(x=0;x<GTempCount && !found;x++)
	{
		if(GTemps[x].weapon==wnt[wn_cur])
		{
			AddGroup(&GTemps[x]);
			switch(wn_cur)
			{
			case 0:
				Groups[GroupCount-1].x=Groups[MechIndex].x-cos((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].y=Groups[MechIndex].y-sin((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].z=Groups[MechIndex].z+.5;
				break;
			case 1:
				Groups[GroupCount-1].x=Groups[MechIndex].x+cos((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].y=Groups[MechIndex].y+sin((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].z=Groups[MechIndex].z+.5;
				break;
			case 2:
				Groups[GroupCount-1].x=Groups[MechIndex].x-cos((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].y=Groups[MechIndex].y-sin((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].z=Groups[MechIndex].z-.5;
				break;
			case 3:
				Groups[GroupCount-1].x=Groups[MechIndex].x+cos((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].y=Groups[MechIndex].y+sin((Groups[MechIndex].a-90)/180*M_PI);
				Groups[GroupCount-1].z=Groups[MechIndex].z-.5;
				break;
			}
			Groups[GroupCount-1].a=Groups[MechIndex].a;
			Groups[GroupCount-1].p=Groups[MechIndex].p*2.0;
			Groups[GroupCount-1].Visible=1;
			found=1;
		}
	}

	wn_rpt--;
	if(!wn_rpt)
	{
		wn_cur++;
		if(wn_cur==4)
			wn_cur=0;
	}
}

void CWorld::AddGroup(CGroup* Group)
{
	logtab++;
	log("start AddGroup");

	if(GroupCount>0)
	{
		CGroup* tempGroups;
		if(!(tempGroups=new CGroup[GroupCount]))
			ERR_MEM();
		for(int x=0;x<GroupCount;x++)
			tempGroups[x]=Groups[x];

		delete[]Groups;
		if(!(Groups=new CGroup[GroupCount+1]))
			ERR_MEM();

		for(x=0;x<GroupCount;x++)
			Groups[x]=tempGroups[x];

		delete[]tempGroups;

		logtab++;
		log("flag  deleted tempGroups");
		logtab--;
	}
	else
	{
		if(!(Groups=new CGroup[1]))
			ERR_MEM();
	}

	Groups[GroupCount]=*Group;

	GroupCount++;

	log("end   AddGroup");
	logtab--;
}

void CWorld::DeleteGroup(int index)
{
	if(GroupCount>1)
	{
		CGroup* tempGroups;
		if(!(tempGroups=new CGroup[GroupCount]))
			ERR_MEM();
		for(int x=0;x<GroupCount;x++)
			tempGroups[x]=Groups[x];

		delete[]Groups;
		if(!(Groups=new CGroup[GroupCount-1]))
			ERR_MEM();

		for(x=0;x<index;x++)
			Groups[x]=tempGroups[x];

		for(x=index;x<GroupCount-1;x++)
			Groups[x]=tempGroups[x+1];

		delete[]tempGroups;
	}
	else
		delete[]Groups;

	GroupCount--;
}

double CWorld::GetA(double x,double y)
{
	double a;

	if(x==0)
	{
		if(y>0)
			a=90;
		else if(y<0)
			a=270;
		else
			a=0;
	}
	else
	{
		a=atan(y/x)/M_PI*180;
		if(x<0)
			a+=180;
		else if(y<0)
			a+=360;
	}

	return a;
}

void CWorld::DrawPolygon(CPolygon* Polygon)
{
	logtab++;
	log("start DrawPolygon");

	double dx,dy,dz,a,t,ta,td;
	double* x;
	double* y;
	double* tx;
	double* ty;
	double* tz;

	if(!(x=new double[Polygon->PointCount * 2]))
		ERR_MEM();
	if(!(y=new double[Polygon->PointCount * 2]))
		ERR_MEM();
	if(!(tx=new double[Polygon->PointCount]))
		ERR_MEM();
	if(!(ty=new double[Polygon->PointCount]))
		ERR_MEM();
	if(!(tz=new double[Polygon->PointCount]))
		ERR_MEM();

	logtab++;
	log("flag  allocated temp vars");
	logtab--;

	for(int c=0;c<Polygon->PointCount;c++)
	{
		dx=Polygon->x[c]-cx;
		dy=Polygon->y[c]-cy;
		dz=Polygon->z[c]-cz;

		ta=GetA(dx,dy)-(ca-90);
		td=hypot(dx,dy);
		dx=cos(ta/180*M_PI)*td;
		dy=sin(ta/180*M_PI)*td;

		ta=GetA(dy,dz)-ct;
		td=hypot(dy,dz);
		dy=cos(ta/180*M_PI)*td;
		dz=sin(ta/180*M_PI)*td;

		tx[c]=dx;
		ty[c]=dy;
		tz[c]=dz;
	}

	int pc=0,less,more;
	double sx,sy,sz,nx,nz;

	for(c=0;c<Polygon->PointCount;c++)
	{
		if(ty[c]>wall)
		{
			a=tx[c]/ty[c];
			t=tz[c]/ty[c];

			x[pc]=((a/.5) * (vw/2))+(vw/2);
			y[pc]=(vh/2)-((t/(.5*(vh/vw))) * (vh/2));
			pc++;
		}
		else
		{
			less=c-1;
			if(less==-1)
				less=Polygon->PointCount-1;

			more=c+1;
			if(more==Polygon->PointCount)
				more=0;

			if(ty[less]>wall && ty[more]>wall)
			{
				sx=tx[less]-tx[c];
				sy=ty[less]-ty[c];
				sz=tz[less]-tz[c];
				nx=tx[c]+((sx/sy) * (wall-ty[c]));
				nz=tz[c]+((sz/sy) * (wall-ty[c]));

				a=nx/wall;
				t=nz/wall;

				x[pc]=((a/.5) * (vw/2))+(vw/2);
				y[pc]=(vh/2)-((t/(.5*(vh/vw))) * (vh/2));
				pc++;

				sx=tx[more]-tx[c];
				sy=ty[more]-ty[c];
				sz=tz[more]-tz[c];
				nx=tx[c]+((sx/sy) * (wall-ty[c]));
				nz=tz[c]+((sz/sy) * (wall-ty[c]));

				a=nx/wall;
				t=nz/wall;

				x[pc]=((a/.5) * (vw/2))+(vw/2);
				y[pc]=(vh/2)-((t/(.5*(vh/vw))) * (vh/2));
				pc++;
			}
			else if(ty[less]>wall)
			{
				sx=tx[less]-tx[c];
				sy=ty[less]-ty[c];
				sz=tz[less]-tz[c];
				nx=tx[c]+((sx/sy) * (wall-ty[c]));
				nz=tz[c]+((sz/sy) * (wall-ty[c]));

				a=nx/wall;
				t=nz/wall;

				x[pc]=((a/.5) * (vw/2))+(vw/2);
				y[pc]=(vh/2)-((t/(.5*(vh/vw))) * (vh/2));
				pc++;
			}
			else if(ty[more]>wall)
			{
				sx=tx[more]-tx[c];
				sy=ty[more]-ty[c];
				sz=tz[more]-tz[c];
				nx=tx[c]+((sx/sy) * (wall-ty[c]));
				nz=tz[c]+((sz/sy) * (wall-ty[c]));

				a=nx/wall;
				t=nz/wall;

				x[pc]=((a/.5) * (vw/2))+(vw/2);
				y[pc]=(vh/2)-((t/(.5*(vh/vw))) * (vh/2));
				pc++;
			}
		}
	}

	log("drawing");
	if(pc>0)
	{
		int* points;
		if(!(points=new int[pc * 2]))
			ERR_MEM();
		for(c=0;c<pc;c++)
		{
			points[2 * c]=x[c];
			points[(2 * c) + 1]=y[c];
		}

		setcolor(16);
		setfillstyle(Polygon->fillstyle,Polygon->color);
		fillpoly(pc,points);

		delete[]points;
	}

	delete[]x;
	delete[]y;
	delete[]tx;
	delete[]ty;
	delete[]tz;

	log("end   DrawPolygon");
	logtab--;
}

void CWorld::AddPolygon(CPolygon* Polygon)
{
	logtab++;
	log("start AddPolygon");
	logtab++;

	int index=0;
	double dist=0;//,dist2=0;

	for(int x=0;x<Polygon->PointCount;x++)
		dist+=sqrt(sqr(Polygon->x[x]-cx)+sqr(Polygon->y[x]-cy)+sqr(Polygon->z[x]-cz));
	dist/=Polygon->PointCount;
	Polygon->dist=dist;

	log("flag  start find index");

	for(x=0;x<PolygonCount;x++)
	{
		//dist2=0;
		//for(int y=0;y<Polygons[x].PointCount;y++)
		//	dist2+=sqrt(sqr(Polygons[x].x[y]-cx)+sqr(Polygons[x].y[y]-cy)+sqr(Polygons[x].z[y]-cz));
		//dist2/=Polygons[x].PointCount;
		if(dist>Polygons[x].dist)
		{
			index=x;
			break;
		}
		else
			index=x+1;
	}

	log("flag  end   find index");

	if(PolygonCount>0)
	{
		//cout<<"index "<<index<<" x "<<x<<" pc "<<PolygonCount<<endl;

		logtab++;
		char* b=new char[6];

		itoa(PolygonsSize,b,10);
		log(b);

		if(PolygonCount<PolygonsSize)
		{
			log("flag  using preallocated space");

			for(x=PolygonCount+1;x>index;x--)
				Polygons[x]=Polygons[x-1];

			log("flag  shifted Polygons past index");

			Polygons[index]=*Polygon;

			//logtab++;
			log("flag  used preallocated space");
			logtab--;
		}
		else
		{
			log("flag  resizing");

			CPolygon* tempPolygons;
			if(!(tempPolygons=new CPolygon[PolygonCount]))
				ERR_MEM();
			for(x=0;x<PolygonCount;x++)
				tempPolygons[x]=Polygons[x];

			log("flag  done  copy temp");

			delete[]Polygons;

			log("flag  deleted Polygons");

			if(!(Polygons=new CPolygon[PolygonCount+11]))
				ERR_MEM();
			PolygonsSize=PolygonCount+11;

			log("flag  done  Polygons resize");

			for(x=0;x<index;x++)
				Polygons[x]=tempPolygons[x];

			Polygons[index]=*Polygon;

			for(x=index;x<PolygonCount;x++)
				Polygons[x+1]=tempPolygons[x];

			log("flag  done  Polygons recopy");

			delete[]tempPolygons;

			log("flag  deleted tempPolygons");
			log("resized");
			logtab--;
		}
	}
	else
	{
		log("flag  initializing Polygons");
		if(PolygonsSize==0)
		{
			if(!(Polygons=new CPolygon[11]))
				ERR_MEM();
			PolygonsSize=11;
		}
		Polygons[0]=*Polygon;

		logtab++;
		log("flag  initialized Polygons");
		logtab--;
	}

	PolygonCount++;

	logtab--;
	log("end   AddPolygon");
	logtab--;
}

void CWorld::DrawPolygons()
{
	log("start DrawPolygons");
	logtab++;

	cleardevice();
	for(int x=0;x<PolygonCount;x++)
		DrawPolygon(&Polygons[x]);

	logtab--;
	log("end   DrawPolygons");
}

void main()
{
	logtab++;
	clearlog();
	log("start main");

	int gdriver=VGA, gmode=VGAMED;

	initgraph(&gdriver,&gmode,"");

	setaspectratio(1,1);
	world.page=0;

	world.vw=getmaxx();
	world.vh=getmaxy();

	world.cw=atan(.5)/M_PI*360.0;
	world.ch=atan((world.vh/world.vw)/2.0)/M_PI*360.0;

	mh=5;

	world.LoadGroups("world.txt",9);

	world.Groups[MechIndex].a=90;
	world.Groups[MechIndex].t=0;

	world.Groups[MechIndex].x=0;
	world.Groups[MechIndex].y=-100;
	world.Groups[MechIndex].z=5;

	//world.ca=90;
	//world.ct=-10;

	//world.cx=0;
	//world.cy=-100;
	//world.cz=5;

	MoveMech();

	//setcolor(15);
	//world.Draw();

	int key=-1,keym=-1;
	double dx,dy;
	union REGS regs;

	do
	{
		if(bioskey(1))
			key=bioskey(0);
		else
			key=-1;

		keym=_bios_keybrd(_NKEYBRD_SHIFTSTATUS);
		if(keym & Mod_RightShift)
			world.Fire();

		dx=0;
		dy=0;

		switch(key)
		{
		case Key_Up:
			world.ct++;
			//dx=cos(world.ca/180*M_PI);
			//dy=sin(world.ca/180*M_PI);
			break;
		case Key_Down:
			world.ct--;
			//dx=-cos(world.ca/180*M_PI);
			//dy=-sin(world.ca/180*M_PI);
			break;
		case Key_NumPad4:
			dx=cos((world.ca+90)/180*M_PI);
			dy=sin((world.ca+90)/180*M_PI);
			break;
		case Key_NumPad6:
			dx=-cos((world.ca+90)/180*M_PI);
			dy=-sin((world.ca+90)/180*M_PI);
			break;
		case Key_Tilde:
			sper=0;
			break;
		case Key_1:
			sper=.1;
			break;
		case Key_2:
			sper=.2;
			break;
		case Key_3:
			sper=.3;
			break;
		case Key_4:
			sper=.4;
			break;
		case Key_5:
			sper=.5;
			break;
		case Key_6:
			sper=.6;
			break;
		case Key_7:
			sper=.7;
			break;
		case Key_8:
			sper=.8;
			break;
		case Key_9:
			sper=.9;
			break;
		case Key_0:
			sper=1;
			break;
		case Key_Backspace:
			sper=-.25;
			break;
		case Key_Pos:
			sper+=.02;
			break;
		case Key_Neg:
			sper-=.02;
			break;
		case Key_PageUp:
			mh++;
			break;
		case Key_PageDown:
			mh--;
			break;
		case Key_Left:
			world.ca+=2;
			break;
		case Key_Right:
			world.ca-=2;
			break;
		case Key_NumPad8:
			world.ct++;
			break;
		case Key_NumPad2:
			world.ct--;
			break;
		case Key_A:
			world.Groups[0].a++;
			break;
		case Key_O:
			OptionsMenu();
			break;
		case Key_Z:	//zoom radar in
			opt_radarradius--;
			break;
		case Key_X:	//zoom radar out
			opt_radarradius++;
		}

		wn_fdc++;
		if(wn_fdc==wn_fd[wn_cur])
		{
			if(wn_rpt>0)
				world.Fire();
			wn_fdc=0;
		}

		regs.x.ax=0x0B;
		int86(0x33,&regs,&regs);
		world.Groups[MechIndex].a-=(int)regs.x.cx/10.0;
		world.Groups[MechIndex].p-=(int)regs.x.dx/10.0;
		//world.ca-=(int)regs.x.cx/10.0;
		//world.ct-=(int)regs.x.dx/10.0;

		regs.x.ax=6;
		regs.x.bx=0;	//left mouse button
		int86(0x33,&regs,&regs);
		if(regs.x.bx)
			world.Fire();

		world.Groups[MechIndex].x+=dx;
		world.Groups[MechIndex].y+=dy;
		//world.cx+=dx;
		//world.cy+=dy;

		if(m_heat>0)
			m_heat--;
		MoveMech();
		MoveObjects();
		//setcolor(0);
		world.Draw();
		//delay(1);	//prevents flicker
	}while(key!=Key_Esc);

	closegraph();

#ifdef OPTSOUND
	nosound();
#endif

	log("end   main");
	logtab--;
}

void OptionsMenu()
{
	DrawOptions(0);
	int key=-1,sel=0,OptionCount=1;

	do
	{
		if(bioskey(1))
		{
			key=bioskey(0);
		}
		else
			key=-1;

		switch(key)
		{
		case Key_Up:
			if(sel<OptionCount)
				sel++;
			break;
		case Key_Down:
			if(sel>0)
				sel--;
			break;
		case Key_PageUp:
		case Key_Right:
			switch(sel)
			{
			case 0:
				opt_radartype=1;
			}
			break;
		case Key_PageDown:
		case Key_Left:
			switch(sel)
			{
			case 0:
				opt_radartype=0;
			}
		}
		if(key!=-1)
			DrawOptions(sel);
	}while(key!=283);
}

void DrawOptions(int sel)
{
	int th=textheight("O");
	setcolor(15);
	setfillstyle(1,0);
	bar(0,0,102+textwidth("Stationary\\Rotates"),th+4);
	rectangle(0,0,102+textwidth("Stationary\\Rotates"),th+4);

	if(sel==0)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(2,2,"Radar Type:");
	if(!opt_radartype)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(102,2,"Stationary");
	setcolor(15);
	outtextxy(102+textwidth("Stationary"),2,"\\");
	if(opt_radartype)
		setcolor(3);
	else
		setcolor(15);
	outtextxy(102+textwidth("Stationary\\"),2,"Rotates");
}

void CWorld::LoadGroups(char* file,int length)
{
	fstream f;

	/*/start test code
	CString* names;
	struct ffblk ffblk;
	findfirst("a:\\turbo\\*.txt",&ffblk,0);
	cout<<"1 "<<ffblk.ff_name<<endl;
	names[0].text=ffblk.ff_name;
	int x=0;
	while(findnext(&ffblk)!=-1)
	{
		cout<<x+2<<" "<<ffblk.ff_name<<endl;
		names[x+1]=ffblk.ff_name;
		x++;
	}
	int filenum;
	cin>>filenum;

	char fn[22]="a:\\turbo\\";
	f.open("a:\\turbo\\"+*names[filenum-1],ios::in,filebuf::openprot);
	//end test code*/

	int x;
	f.open(worldfile,ios::in,filebuf::openprot);
	char b[256];

	f>>b;
	GroupCount=atoi(b);
	if(!(Groups=new CGroup[GroupCount]))
		ERR_MEM();

	for(x=0;x<GroupCount;x++)
	{
		f>>b;
		Groups[x].Visible=atoi(b);
		f>>b;
		Groups[x].radar=atoi(b);
		f>>b;
		Groups[x].ident=atoi(b);
		switch(Groups[x].ident)
		{
		case 0:	//player mech
			MechIndex=x;
			break;
		case 4: //weapon
			f>>b;
			Groups[x].weapon=atoi(b);
			if(!(Groups[x].Tags=new int[1]))
				ERR_MEM();
			Groups[x].TagCount=1;
			Groups[x].Tags[0]=0;
			break;
		}

		f>>b;
		Groups[x].PolygonCount=atoi(b);
		if(!(Groups[x].Polygons=new CPolygon[Groups[x].PolygonCount]))
			ERR_MEM();
		for(int y=0;y<Groups[x].PolygonCount;y++)
		{
			switch(Groups[x].ident)
			{
			case 0: //player mech
				Groups[x].Polygons[y].fillstyle=1;
				break;
			default:
				Groups[x].Polygons[y].fillstyle=1;
			}

			f>>b;
			Groups[x].Polygons[y].color=atoi(b);
			f>>b;
			Groups[x].Polygons[y].PointCount=atoi(b);
			if(!(Groups[x].Polygons[y].x=new double[Groups[x].Polygons[y].PointCount]))
				ERR_MEM();
			if(!(Groups[x].Polygons[y].y=new double[Groups[x].Polygons[y].PointCount]))
				ERR_MEM();
			if(!(Groups[x].Polygons[y].z=new double[Groups[x].Polygons[y].PointCount]))
				ERR_MEM();
			for(int z=0;z<Groups[x].Polygons[y].PointCount;z++)
			{
				f>>b;
				Groups[x].Polygons[y].x[z]=atof(b);
				f>>b;
				Groups[x].Polygons[y].y[z]=atof(b);
				f>>b;
				Groups[x].Polygons[y].z[z]=atof(b);
			}
		}
	}

	//GTemps
	f>>b;
	GTempCount=atoi(b);
	if(!(GTemps=new CGroup[GTempCount]))
		ERR_MEM();
	for(x=0;x<GTempCount;x++)
	{
		f>>b;
		GTemps[x].Visible=atoi(b);
		f>>b;
		GTemps[x].radar=atoi(b);
		f>>b;
		GTemps[x].ident=atoi(b);
		if(GTemps[x].ident==4)//weapon
		{
			f>>b;
			GTemps[x].weapon=atoi(b);
			if(!(GTemps[x].Tags=new int[1]))
				ERR_MEM();
			GTemps[x].TagCount=1;
			GTemps[x].Tags[0]=0;
		}

		f>>b;
		GTemps[x].PolygonCount=atoi(b);
		if(!(GTemps[x].Polygons=new CPolygon[GTemps[x].PolygonCount]))
			ERR_MEM();
		for(int y=0;y<GTemps[x].PolygonCount;y++)
		{
			switch(GTemps[x].ident)
			{
			case 0: //player mech
				GTemps[x].Polygons[y].fillstyle=1;
				break;
			default:
				GTemps[x].Polygons[y].fillstyle=1;
			}

			f>>b;
			GTemps[x].Polygons[y].color=atoi(b);
			f>>b;
			GTemps[x].Polygons[y].PointCount=atoi(b);
			if(!(GTemps[x].Polygons[y].x=new double[GTemps[x].Polygons[y].PointCount]))
				ERR_MEM();
			if(!(GTemps[x].Polygons[y].y=new double[GTemps[x].Polygons[y].PointCount]))
				ERR_MEM();
			if(!(GTemps[x].Polygons[y].z=new double[GTemps[x].Polygons[y].PointCount]))
				ERR_MEM();
			for(int z=0;z<GTemps[x].Polygons[y].PointCount;z++)
			{
				f>>b;
				GTemps[x].Polygons[y].x[z]=atof(b);
				f>>b;
				GTemps[x].Polygons[y].y[z]=atof(b);
				f>>b;
				GTemps[x].Polygons[y].z[z]=atof(b);
			}
		}
	}
	f.close();
	delete[]b;
}

void MoveMech()
{
	world.Groups[MechIndex].x+=cos(world.Groups[MechIndex].a/180*M_PI)*sper;
	world.Groups[MechIndex].y+=sin(world.Groups[MechIndex].a/180*M_PI)*sper;
	wdist+=fabs(sper);
	wdist=fmod(wdist,sdist);
	if(sper==0 && wdist>.1)
		wdist-=.1;
	else if(sper==0)
		wdist=0;

	world.Groups[MechIndex].z=mh+(sin(wdist/sdist*M_PI)*sheight);
	world.cx=world.Groups[MechIndex].x;//+(cos(world.Groups[MechIndex].a/180*M_PI)*1.0);
	world.cy=world.Groups[MechIndex].y;//+(sin(world.Groups[MechIndex].a/180*M_PI)*1.0);
	world.cz=world.Groups[MechIndex].z;
	world.ca=world.Groups[MechIndex].a;
	world.ct=world.Groups[MechIndex].p;
}

void MoveObjects()
{
	double dx,dy,dz,dt,dist=1000,dist2;

	for(int x=0;x<world.GroupCount;x++)
	{
		if(world.Groups[x].weapon>=0)
		{
			dt=cos(world.Groups[x].p/180*M_PI);
			dz=sin(world.Groups[x].p/180*M_PI);
			dx=cos(world.Groups[x].a/180*M_PI) * dt;
			dy=sin(world.Groups[x].a/180*M_PI) * dt;
			world.Groups[x].x+=dx * 10.0;
			world.Groups[x].y+=dy * 10.0;
			world.Groups[x].z+=dz * 10.0;
			if((world.Groups[x].Tags[0]++)>100)
				world.DeleteGroup(x);
			else
			{
				dist2=pow(sqr(world.Groups[x].x-world.cx)+sqr(world.Groups[x].y-world.cy)+sqr(world.Groups[x].z-world.cz),.5)*5.0;
				if(dist2<dist)
					dist=dist2;
			}
		}
	}
#ifdef OPTSOUND
	if(dist<1000)
		sound(1000-dist);
	else
		nosound();
#endif
}

double sqr(double x)
{
	return x * x;
}