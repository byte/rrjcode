-- wordpress getting you down with weird characters after a MySQL upgrade? this fixes it

UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬Å“', 'â€œ');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬Â', 'â€');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬â„¢', 'â€™');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬Ëœ', 'â€˜');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬â€', 'â€“');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬â€œ', 'â€”');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬Â¢', '-');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Ã¢â‚¬Â¦', 'â€¦');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'Â', '');
UPDATE wp_posts SET post_content = REPLACE(post_content, 'ï»¿', '');

UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬Å“', 'â€œ');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬Â', 'â€');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬â„¢', 'â€™');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬Ëœ', 'â€˜');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬â€', 'â€“');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬â€œ', 'â€”');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬Â¢', '-');
UPDATE wp_comments SET comment_content = REPLACE(comment_content, 'Ã¢â‚¬Â¦', 'â€¦');
