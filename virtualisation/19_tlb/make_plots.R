library(readr)
library(ggplot2)

num_trials <- commandArgs(trailingOnly=TRUE)[1]

png_filenames = character()

for (tlb_outfile in paste0('tlb.', num_trials, '_trials.out', 1:5)) {
    df <- read_csv(file = tlb_outfile,
                   col_types = cols(usecs = 'i', numpages = 'i'))
    df$nsecs_per_access <- 1000 * df$usecs / (10000 * df$numpages)
    
    png_filename = paste0(tlb_outfile, '.png')
    
    png(filename = png_filename)
    print(ggplot(df, aes(x = as.factor(numpages), y = nsecs_per_access)) +
              geom_line(aes(group = 1)) + geom_point() +
              labs(title = tlb_outfile,
                   x = 'Number of pages', y = 'Time per access (ns)'))
    dev.off()
    
    png_filenames = c(png_filenames, png_filename)
}

# Output filenames to stdout so that bash script can `open` them
write(png_filenames, file = '', ncolumns = length(png_filenames))
