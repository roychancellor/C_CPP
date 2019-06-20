#define ANIMATION 1
#define FULL_SCREEN 0
#define SPRING 0
#define DAMP   1
#define AMPL   2
#define MASS   3
#define FREQ   4
#define INVERTED  1
#define REGULAR   0
#define START_COL 22
#define FILE_NAME        3
#define DIRECTORY_NAME   FILE_NAME + 2
#define RESP_TYPE_PROMPT FILE_NAME + 9
#define CONNECT_POINTS   FILE_NAME + 19
#define Y_AXIS_PROMPT    FILE_NAME + 14
#define TAB_COL 5
#define INDENT_COL START_COL + TAB_COL
#define P 1
#define M 2
#define B 3
#define T 4
#define D 1
#define V 0
#define TIME_EPS .00000001
#define XFAC 1.4
#define YFAC 1.4

struct pend
{
	int    pts_per_cycle;
	int    pend_type;
	double poin_phase;
	double wd;
	double theta_0;
	double omeg_0;
	double tmin;
	double tmax;
	double q;
	double g;
	double c_I;
	double kt_I;
	double mg_I;
	double tho;
	char   sim_fname[MAXSTR];
	char   resp_fname[MAXSTR];
	char   grph_fname[MAXSTR];
	char   pend_fname[MAXSTR];
};
struct bif
{
	char phase_var[20];
	char bif_par[20];
	double delta_bp;
	double bp_i;
	double bp_f;
};
struct calc_params
{
	double c_I;
	double tho;
	double wd;
	double dampmax;
	double amplmax;
	double freqmax;
	double dc_I;
	double dtho;
	double dwd;
	double q;
	double g;
	double dq;
	double dg;
};
struct vport
{
	int tlx;
	int tly;
	int brx;
	int bry;
	int adapt;
	int pend_type;
};
struct sim_control
{
	int   show_graphics;
	int   save_flag;
	int   pt_color;
	int   time_row;
	int   time_col;
	int   sim_type;
	int   amp_row;
	int   amp_col;
	int   damp_row;
	int   damp_col;
	int   freq_row;
	int   freq_col;
};
double accel( double, double, double, double [], int );
void runge( double, double, double *, double *,
					double, double, double [], int );
void get_pend_data( struct pend *, char [] );
void get_bifur_data( struct bif *, char [] );
void display_message( int, struct sim_control );
int bifeditr( char [] );
void setup_anima( void );
void setup_phs_pln( void );
void get_vport_info( struct vport *, struct vport * );
void calc_points( struct pend, struct vport, struct vport );
void draw_pend( double, double, struct axis_data, struct vport, int );
void draw_phase( double, double, struct axis_data, double, double,
					  struct vport );
void prog_control( short, struct sim_control *, FILE *, struct pend );
void aget_graph_data( struct axis_data *, char [] );
void asetup_colors( struct axis_data * );
void aset_mode_and_rectangle( struct axis_data, int );
void adraw_graph_axes( struct axis_data, int, int, int );
void awrite_titles( struct axis_data, int );
void afont_stuff( void );
void setup_animation( char [], int, struct axis_data * );
