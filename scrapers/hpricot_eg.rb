require 'rubygems'
require 'hpricot'
require 'open-uri'


doc = Hpricot(open('http://www.maybank2u.com.my/mbbfrx/gold_rate.htm'))
puts doc.search("html/")
rows = []
doc.search("/html/body//p/html").each do |row|
  cells = []
  (row/"td").each do |cell|
  #   puts cell.gsub(/<\/?[^>]*>/, "")
  puts type cell
end
end
