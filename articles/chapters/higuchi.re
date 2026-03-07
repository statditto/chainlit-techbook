
= 章タイトル

ChainlitのUIを@<ruby>{癖, へき}に染める

== はじめに
LLMが一般社会に急速に浸透し、我々の生活やワークスタイルは大きく変化しました。日々の献立から専門的な分野の調査分析まで、普段使う言葉で対話しながら解決できるLLMは、もはや日常になくてはならないものになっています。まるで、先人たちがSF作品で描いたような世界が現実になりつつあります。
特にゲームに絞ると、このような人工知能との関わりを描いた作品は数多く思い当たります。「ナビ」と呼ばれる人工知能を使ってインターネットウイルスを駆逐したり、2つのポータルと人間の知恵を駆使して暴走した人工知能と対峙したり。最近では、人工知能と共闘してリモートで「ホロウ」と呼ばれる危険地帯を探索する便利屋を扱ったソーシャルゲームも登場しました。このような作品に触れてきた方は、もしかしたらLLMに対し、単なるツールを超えた人格としての親近感を覚える方もいるのではないでしょうか。少なくとも私はそうです。
このような作品では、世界観に合わせて近未来的なUIが導入されることが多く、その鮮烈なカラーリングと演出に心を躍らせたものです。他方、大衆向け・ことビジネス向けサービスのUIに目を向けると、シンプルさ・わかりやすさが重視されます。ChatGPTやClaude Code、Chainlitの標準デザインもシンプルな構造ですよね。実際、シンプルな方がビジネスのさまざまな場面でノイズになりにくいですし、操作のわかりやすさという点でも理にかなっています。しかし頭では理解しながらも、LLMというSF作品の夢を具現化したような存在が現れた今、それを扱うUIも創作の典型に寄せてみたら、日々の業務がちょっと楽しくなるのでは？という思いを常に抱えていました。そんな時にdittoさんにこの記事執筆のお話をいただき、実際に手を動かして作ってみることにしました。
フロントエンドの開発の経験はなく、学生時代に静的サイトの装飾を多少扱った程度ですが、そのレベルでもできる範囲で実装してみます。

== この記事の取扱範囲と資料について
=== 取扱範囲・取り扱わない範囲
この記事ではChainlitに対するCSS, JSの適用による要素の変更を取り扱います。
想定レベルは「普段pythonやその他の言語を利用して分析・開発を行っているひと」
下記の項目は取り扱いません。
- CSS, JSの専門的な解説
- tailwind(ChainlitはShadcn/tailwindベースですが今回は利用しません)

=== 参考資料について
題材の性質上、この記事にはカラーで見た方が箇所がたびたび登場します。ただ、書籍は白黒印刷のため、下記のサイトに参考資料を掲載しました。
記事内でも必要な箇所にサイトに遷移するQRコードを掲載しますので、スマホ片手にご覧ください。
link
QR Code

== ChainlitのUI変更の仕様を知る
ChainlitはLLMとの会話アプリを簡単に実装することを主眼に作られています。フロントエンド開発の知識が乏しくてもアプリを構築できるよう、標準で使いやすいチャットUIが用意されています。これらはReact + Tailwind + shadcn/uiの構成でChainlit内部に実装されており、通常のプロジェクト作成時にはフロントエンドのソースコードはプロジェクトディレクトリに生成されません。
ただし、簡単なデザイン変更はサポートされています。具体的には、config.tomlに装飾用のファイル（css, js, faviconなど）のpathを書き込むことで、標準UIのスタイルを上書きできます。
整理すると、HTMLは編集不可、CSSとJSは適用可能という構図です。そのため、*標準UIをCSSで装飾しつつ、必要であればjsでHTMLの要素を編集する*という作戦で実装することにします。

== 目標
実装にあたり、Figmaでトンマナ案とデザイン案を作成しました。カラー版はリンクをご覧ください。
あくまでも案として、実装による多少の差異は許容することにします。

image, QR

また、今回の実装にあたり、円をモチーフにしたロゴを作成しました。最終的にはこのロゴを回転させます。

image

== 変更していこう

=== ロゴを変更する
手始めに、ロゴを変えていきましょう。このロゴはfaviconにも使われます。まず、.chainlitディレクトリと同じ階層にpublicディレクトリを作り、faviconとロゴに使うファイルを配置します。

@<list>{config.yamlとpublic}

その後、config.yamlに移動し、logo_file_urlのコメントアウトを解除します。そして配置したロゴファイルのpathを入力すれば完了です。簡単！ついでに、AIの返答メッセージの横に表示されるアイコンも同じファイルにしておきましょう。
@<list>{config.yamlの編集}

補足として、config.yamlには他にも設定項目があります。この記事では特に扱いませんが、これらを使えば標準UIのデザインをさらにカスタマイズできるでしょう。
@<list>{その他の設定項目例}

=== 見出しやボタンの表記を変える
.chainlitの配下にtranslationsというディレクトリと多数のjsonが自動生成されます。これらは、ChainlitのUIで使用されるラベルやボタン、メニューなどのテキストの翻訳辞書です。これらの翻訳ファイルを編集することで、UIの表示文言を任意の言葉に変更することができます。デフォルトでは「ログイン」「パスワード」など、日本語を使った表記にしてくれているのですが、今回は見た目重視で英単語で表記されるように変更します。

@<list>{ja.jsonの設定}

ここまでで、ロゴと表記を変更できました。今の見た目はこのようになっています。

//image[変更後][seahawk-default-login][scale=0.45]
//image[変更前][seahawk-customized-login][scale=0.45]


=== 静的要素を変更する

==== ログイン画面の変更

==== トーク画面の変更

=== 動的要素を変更する

==== ログイン画面の変更

==== トーク画面の変更

=== 完成！

== 終わりに



画像はarticlesフォルダの下のimagesフォルダに入れて、こんな風に指定すると表示されます。（@<img>{sample-diagram}）

//image[sample-diagram][画像はここに表示される][scale=0.8]{
//}

注釈を書くこともできます。@<fn>{footnote-sample}

//footnote[footnote-sample][注釈はこんな風に表示される]

Re:VIEWの文法について詳しくは、Re:VIEW フォーマットガイドを参照してください。

 * Re:VIEW フォーマットガイド
 ** @<href>{https://github.com/kmuto/review/blob/master/doc/format.ja.md}

//emlist[config.yamlとpublic]
その他のディレクトリは省略
.chainlit
├── translations/ …省略
└── config.yaml

public/
└── logo.svg
//

//emlist[config.yamlの編集][python]
# Load assistant logo directly from URL.
logo_file_url = "/public/logo.svg"

# Load assistant avatar image directly from URL.
default_avatar_file_url = "/public/logo.svg"
//

//emlist[その他の設定項目例][python]
# Custom login page image, relative to public directory or external URL
login_page_image = "/public/custom-background.jpg"

# Custom login page image filter (Tailwind internal filters, no dark/light variants)
login_page_image_filter = "brightness-50 grayscale"
login_page_image_dark_filter = "contrast-200 blur-sm"

# Specify a custom meta image url.
custom_meta_image_url = "https://chainlit-cloud.s3.eu-west-3.amazonaws.com/logo/chainlit_banner.png"
//


//emlist[ja.jsonの設定][json]
    "auth": {
        "login": {
            "title": "SYSTEM LOGIN",
            "form": {
                "email": {
                    "label": "USER ID",
                    "required": "USER ID IS REQUIRED / USER ID が必要です",
                    "placeholder": "USER ID"
                },
                "password": {
                    "label": "PASSWORD",
                    "required": "PASSWORD IS REQUIRED / PASSWORD が必要です",
                    "placeholder": "Password"
                },
                "actions": {
                    "signin": "LOGIN"
                },
                "alternativeText": {
                    "or": "\u307e\u305f\u306f"
                }
            },
//