 
// Initialize modules
var gulp = require('gulp');
var sass = require('gulp-sass');

// Sass task: compiles the style.scss file into style.css
gulp.task('sass', function(){
    return gulp.src('itw1.scss')
        .pipe(sass()) // compile SCSS to CSS
        .pipe(gulp.dest('.')); // put final CSS in dist folder
});

// JS task: concatenates and uglifies JS files to script.js
// gulp.task('js', function(){
//     return gulp.src(['app/js/plugins/*.js', 'app/js/*.js'])
//         .pipe(concat('all.js'))
//         .pipe(uglify())
//         .pipe(gulp.dest('dist'));
// });

// Watch task: watch SCSS and JS files for changes
gulp.task('watch', function(){
    gulp.watch('*.scss', gulp.series('sass'));
});

// // Default task
// gulp.task('default', gulp.series('sass', 'js', 'watch'));