struct fft_cfg
{
	char     fft_file_path[MAXSTR];
	char     fft_drv[MAXSTR];
	char     fft_direct[MAXSTR];
	char     fft_fname[MAXSTR];
	char     time_file_path[MAXSTR];
	char     time_drv[MAXSTR];
	char     time_direct[MAXSTR];
	char     time_fname[MAXSTR];
	double   dt;
	double   min_freq_file;
	double   max_freq_file;
	int      nocol;
	int      fft_col;
	int      num_time_pts;
};

int get_data( FILE *, double [], double [], struct fft_cfg );
int get_config( char [], struct fft_cfg * );
int calc_fft( struct fft_cfg );
int ffteditr( char [] );
